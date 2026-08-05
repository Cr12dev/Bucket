#version 330 core

in vec3 v_normal;
in vec2 v_uv;
in vec3 v_world_pos;
in float v_face;

uniform sampler2D u_albedo;
uniform vec4 u_color;
uniform vec3 u_view_pos;
uniform vec3 u_ambient;
uniform vec3 u_fog_color;
uniform float u_fog_start;
uniform float u_fog_end;
uniform float u_fog_enabled;

uniform sampler2D u_shadow_map;
uniform mat4 u_light_view_proj;
uniform float u_shadow_enabled;
uniform float u_shadow_bias;

uniform sampler2D u_mirror_tex;
uniform mat4 u_mirror_view_proj;
uniform float u_mirror_enabled;
uniform float u_reflectivity;

uniform float u_uv_scale;  // 0 = mesh UVs, >0 = automatic triplanar UV

uniform vec4 u_mirror_clip;           // plane (normal, d) for the mirror pass
uniform float u_mirror_clip_enabled;

// PBR-ish map slots (bound to texture units 3..12 by the engine)
uniform sampler2D u_normal_map;
uniform sampler2D u_roughness_map;
uniform sampler2D u_emission_map;
uniform sampler2D u_face_albedo[6];
uniform sampler2D u_metallic_map;
uniform float u_normal_enabled;
uniform float u_roughness_enabled;
uniform float u_emission_enabled;
uniform float u_face_albedo_enabled[6];
uniform float u_metallic_enabled;
uniform vec3 u_emission_color;

struct light_data {
  vec4 type_color;      // x = type (0 dir, 1 point, 2 spot), rgb = color
  vec4 intensity_pos;   // x = intensity, yzw = position
  vec4 range_dir;       // x = range, yzw = direction
  vec4 spot_angles;     // x = inner cos, y = outer cos
};

uniform light_data u_lights[8];
uniform int u_light_count;

out vec4 frag_color;

vec3 calc_light(light_data l, vec3 n, vec3 view_dir, vec3 diff_color, vec3 spec_color, float spec_power, vec3 light_dir)
{
  float diff = max(dot(n, light_dir), 0.0);
  vec3 half_dir = normalize(light_dir + view_dir);
  float spec = pow(max(dot(n, half_dir), 0.0), spec_power);
  return l.type_color.rgb * l.intensity_pos.x * (diff * diff_color + spec * spec_color);
}

// 5x5 PCF soft shadow from the directional light's shadow map.
float calc_shadow(vec3 world_pos)
{
  if (u_shadow_enabled < 0.5) return 1.0;

  vec4 lsp = u_light_view_proj * vec4(world_pos, 1.0);
  if (lsp.w <= 0.0) return 1.0;
  vec3 ndc = lsp.xyz / lsp.w * 0.5 + 0.5;
  if (ndc.x < 0.0 || ndc.x > 1.0 || ndc.y < 0.0 || ndc.y > 1.0 || ndc.z > 1.0) return 1.0;

  float shadow = 0.0;
  vec2 texel = 1.0 / vec2(textureSize(u_shadow_map, 0));
  for (int x = -2; x <= 2; ++x) {
    for (int y = -2; y <= 2; ++y) {
      float d = texture(u_shadow_map, ndc.xy + vec2(x, y) * texel).r;
      shadow += (ndc.z - u_shadow_bias) <= d ? 1.0 : 0.0;
    }
  }
  return shadow / 25.0;
}

// Automatic UV mapping: projects the world position onto the dominant
// normal axis (triplanar), so textures tile correctly on any box without
// needing authored UVs.
vec2 auto_uv(vec3 world_pos, vec3 n, float scale)
{
  n = abs(n);
  if (n.y >= n.x && n.y >= n.z) return world_pos.xz * scale;   // top/bottom
  if (n.x >= n.z) return world_pos.zy * scale;                 // +-x faces
  return world_pos.xy * scale;                                 // +-z faces
}

// Per-face albedo override, keyed by the box face slot (0..5).
vec3 sample_albedo(vec2 uv)
{
  int face = clamp(int(v_face + 0.5), 0, 5);
  if (u_face_albedo_enabled[face] > 0.5)
    return texture(u_face_albedo[face], uv).rgb;
  return texture(u_albedo, uv).rgb;
}

// Tangent frame for axis-aligned faces (boxes), good enough for normal maps.
vec3 tangent_from_normal(vec3 n)
{
  vec3 up = abs(n.y) > 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
  return normalize(cross(up, n));
}

void main()
{
  if (u_mirror_clip_enabled > 0.5 &&
      dot(vec4(v_world_pos, 1.0), u_mirror_clip) < 0.0) {
    discard;  // geometry on the wrong side of the mirror plane
  }

  vec3 n = normalize(v_normal);
  vec2 uv = u_uv_scale > 0.0 ? auto_uv(v_world_pos, n, u_uv_scale) : v_uv;
  vec3 albedo = sample_albedo(uv) * u_color.rgb;
  vec3 view_dir = normalize(u_view_pos - v_world_pos);

  // normal mapping (axis-aligned TBN)
  if (u_normal_enabled > 0.5) {
    vec3 map_n = texture(u_normal_map, uv).rgb * 2.0 - 1.0;
    vec3 tangent = tangent_from_normal(n);
    vec3 bitangent = cross(n, tangent);
    n = normalize(mat3(tangent, bitangent, n) * map_n);
  }

  // roughness drives the Blinn-Phong specular power
  float roughness = u_roughness_enabled > 0.5 ? texture(u_roughness_map, uv).r : 0.5;
  float spec_power = mix(256.0, 2.0, roughness);

  // metallic: 0 = dielectric (plastic/paint), 1 = metal.
  // Metals lose diffuse light, get albedo-colored specular (f0) and
  // albedo-tinted reflections.
  float metallic = u_metallic_enabled > 0.5 ? texture(u_metallic_map, uv).r : 0.0;
  vec3 f0 = mix(vec3(0.04), albedo, metallic);
  vec3 diff_color = albedo * (1.0 - metallic);

  vec3 result = u_ambient * diff_color;

  for (int i = 0; i < u_light_count; ++i) {
    light_data l = u_lights[i];
    int type = int(l.type_color.x + 0.5);

    if (type == 0) {
      // directional (sun) with soft shadow
      vec3 light_dir = normalize(-l.range_dir.yzw);
      result += calc_light(l, n, view_dir, diff_color, f0, spec_power, light_dir)
              * calc_shadow(v_world_pos);
    } else if (type == 1) {
      // point
      vec3 to_light = l.intensity_pos.yzw - v_world_pos;
      float dist = length(to_light);
      vec3 light_dir = to_light / dist;
      float atten = max(1.0 - dist / l.range_dir.x, 0.0);
      atten *= atten;
      result += calc_light(l, n, view_dir, diff_color, f0, spec_power, light_dir) * atten;
    } else {
      // spot
      vec3 to_light = l.intensity_pos.yzw - v_world_pos;
      float dist = length(to_light);
      vec3 light_dir = to_light / dist;
      float atten = max(1.0 - dist / l.range_dir.x, 0.0);
      atten *= atten;
      float cos_angle = dot(light_dir, normalize(-l.range_dir.yzw));
      float spot = smoothstep(l.spot_angles.y, l.spot_angles.x, cos_angle);
      result += calc_light(l, n, view_dir, diff_color, f0, spec_power, light_dir) * atten * spot;
    }
  }

  // emission map
  if (u_emission_enabled > 0.5) {
    result += texture(u_emission_map, uv).rgb * u_emission_color;
  }

  // planar reflection (mirror texture rendered from the reflected camera).
  // Metals reflect their own albedo color, dielectrics reflect white.
  if (u_mirror_enabled > 0.5 && u_reflectivity > 0.0) {
    vec4 clip = u_mirror_view_proj * vec4(v_world_pos, 1.0);
    vec2 muv = clip.xy / clip.w * 0.5 + 0.5;
    vec3 reflected = texture(u_mirror_tex, muv).rgb;
    reflected *= mix(vec3(1.0), albedo, metallic);
    result = mix(result, reflected, u_reflectivity);
  }

  // exponential fog
  float dist = length(u_view_pos - v_world_pos);
  float fog_factor = smoothstep(u_fog_start, u_fog_end, dist) * u_fog_enabled;
  result = mix(result, u_fog_color, fog_factor);

  // Reinhard tonemapping + gamma correction: keeps HDR light intensities
  // from blowing out and gives the image a natural, non-washed look.
  result = result / (result + 1.0);
  result = pow(result, vec3(1.0 / 2.2));

  frag_color = vec4(result, u_color.a);
}
