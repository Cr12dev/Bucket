#version 330 core

in vec2 v_uv;

uniform vec3 u_color_a;
uniform vec3 u_color_b;
uniform float u_scale;

out vec4 frag_color;

void main()
{
  vec2 cell = floor(v_uv * u_scale);
  float even = mod(cell.x + cell.y, 2.0);
  vec3 color = mix(u_color_a, u_color_b, even);
  frag_color = vec4(color, 1.0);
}
