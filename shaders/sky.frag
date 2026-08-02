#version 330 core

in vec3 v_ray;

uniform vec3 u_camera_pos;
uniform vec3 u_sun_dir;

out vec4 frag_color;

void main()
{
  vec3 dir = normalize(v_ray - u_camera_pos);
  float up = dir.y;

  // atmosphere gradient (approximate Rayleigh scattering)
  vec3 top     = vec3(0.13, 0.38, 0.88);
  vec3 horizon = vec3(0.72, 0.83, 0.93);
  vec3 ground  = vec3(0.42, 0.38, 0.32);

  float t = clamp(up * 0.5 + 0.5, 0.0, 1.0);
  vec3 col = mix(horizon, top, pow(t, 1.4));

  // sun disc + glow
  float sun_angle = max(dot(dir, normalize(u_sun_dir)), 0.0);
  float disc = pow(sun_angle, 900.0);
  float glow = pow(sun_angle, 10.0) * 0.55;
  col += vec3(1.0, 0.92, 0.75) * (disc * 2.2 + glow);

  // ground below horizon
  col = mix(ground, col, smoothstep(-0.02, 0.05, up));

  // horizon haze (lighter for a clear daytime sky)
  col = mix(col, horizon, exp(-abs(up) * 6.0) * 0.15);

  frag_color = vec4(col, 1.0);
}
