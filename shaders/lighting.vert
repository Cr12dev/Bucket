#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in float a_face;

uniform mat4 u_view_proj;
uniform mat4 u_model;

out vec3 v_normal;
out vec2 v_uv;
out vec3 v_world_pos;
out float v_face;

void main()
{
  gl_Position = u_view_proj * u_model * vec4(a_position, 1.0);
  v_world_pos = (u_model * vec4(a_position, 1.0)).xyz;
  v_normal = mat3(transpose(inverse(u_model))) * a_normal;
  v_uv = a_uv;
  v_face = a_face;
}
