#version 330 core

in vec3 v_normal;
in vec2 v_uv;

uniform sampler2D u_albedo;
uniform vec4 u_color;

out vec4 frag_color;

void main()
{
  vec4 tex = texture(u_albedo, v_uv);
  frag_color = tex * u_color;
}
