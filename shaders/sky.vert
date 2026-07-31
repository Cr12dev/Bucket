#version 330 core

layout (location = 0) in vec3 a_position;

uniform mat4 u_inv_view_proj;

out vec3 v_ray;

void main()
{
  vec4 world = u_inv_view_proj * vec4(a_position, 1.0);
  v_ray = world.xyz / world.w;
  gl_Position = vec4(a_position, 1.0);
}
