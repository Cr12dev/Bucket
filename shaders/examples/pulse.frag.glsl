#version 330 core

in vec2 v_uv;

uniform float u_time;

out vec4 frag_color;

void main()
{
  vec2 center = v_uv - 0.5;
  float dist = length(center);
  float ring = sin(dist * 40.0 - u_time * 4.0) * 0.5 + 0.5;
  float fade = smoothstep(0.5, 0.0, dist);
  frag_color = vec4(vec3(ring * fade), 1.0);
}
