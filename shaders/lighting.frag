#version 330 core

in vec3 v_normal;
in vec2 v_uv;
in vec3 v_world_pos;

uniform sampler2D u_albedo;
uniform vec4 u_color;
uniform vec3 u_view_pos;
uniform vec3 u_ambient;
uniform vec3 u_fog_color;
uniform float u_fog_start;
uniform float u_fog_end;

struct light_data {
  vec4 type_color;      // x = type (0 dir, 1 point, 2 spot), rgb = color
  vec4 intensity_pos;   // x = intensity, yzw = position
  vec4 range_dir;       // x = range, yzw = direction
  vec4 spot_angles;     // x = inner cos, y = outer cos
};

uniform light_data u_lights[8];
uniform int u_light_count;

out vec4 frag_color;

vec3 calc_light(light_data l, vec3 n, vec3 view_dir, vec3 albedo, float spec_power, vec3 light_dir)
{
  float diff = max(dot(n, light_dir), 0.0);
  vec3 half_dir = normalize(light_dir + view_dir);
  float spec = pow(max(dot(n, half_dir), 0.0), spec_power);
  return l.type_color.rgb * l.intensity_pos.x * (diff + spec) * albedo;
}

void main()
{
  vec3 albedo = texture(u_albedo, v_uv).rgb * u_color.rgb;
  vec3 n = normalize(v_normal);
  vec3 view_dir = normalize(u_view_pos - v_world_pos);
  float spec_power = 48.0;

  vec3 result = u_ambient * albedo;

  for (int i = 0; i < u_light_count; ++i) {
    light_data l = u_lights[i];
    int type = int(l.type_color.x + 0.5);

    if (type == 0) {
      // directional
      vec3 light_dir = normalize(-l.range_dir.yzw);
      result += calc_light(l, n, view_dir, albedo, spec_power, light_dir);
    } else if (type == 1) {
      // point
      vec3 to_light = l.intensity_pos.yzw - v_world_pos;
      float dist = length(to_light);
      vec3 light_dir = to_light / dist;
      float atten = max(1.0 - dist / l.range_dir.x, 0.0);
      atten *= atten;
      result += calc_light(l, n, view_dir, albedo, spec_power, light_dir) * atten;
    } else {
      // spot
      vec3 to_light = l.intensity_pos.yzw - v_world_pos;
      float dist = length(to_light);
      vec3 light_dir = to_light / dist;
      float atten = max(1.0 - dist / l.range_dir.x, 0.0);
      atten *= atten;
      float cos_angle = dot(light_dir, normalize(-l.range_dir.yzw));
      float spot = smoothstep(l.spot_angles.y, l.spot_angles.x, cos_angle);
      result += calc_light(l, n, view_dir, albedo, spec_power, light_dir) * atten * spot;
    }
  }

  // exponential fog
  float dist = length(u_view_pos - v_world_pos);
  float fog_factor = smoothstep(u_fog_start, u_fog_end, dist);
  result = mix(result, u_fog_color, fog_factor);

  frag_color = vec4(result, u_color.a);
}
