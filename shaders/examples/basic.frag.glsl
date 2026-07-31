#version 330 core

in vec3 v_world_pos;
in vec3 v_normal;
in vec2 v_uv;

uniform sampler2D u_albedo;
uniform vec4 u_color;
uniform vec3 u_camera_pos;

out vec4 frag_color;

void main()
{
  vec3 normal = normalize(v_normal);

  vec3 light_dir = normalize(vec3(0.5, 1.0, 0.3));
  vec3 light_color = vec3(1.0, 0.95, 0.85);
  float ambient = 0.15;
  float diff = max(dot(normal, light_dir), 0.0);

  vec3 view_dir = normalize(u_camera_pos - v_world_pos);
  vec3 reflect_dir = reflect(-light_dir, normal);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32.0);

  vec3 base = texture(u_albedo, v_uv).rgb * u_color.rgb;
  vec3 lighting = (ambient + diff) * light_color + spec * light_color;
  frag_color = vec4(base * lighting, u_color.a);
}
