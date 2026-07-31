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
  vec3 top     = vec3(0.08, 0.25, 0.65);
  vec3 horizon = vec3(0.68, 0.78, 0.86);
  vec3 ground  = vec3(0.30, 0.28, 0.26);

  float t = clamp(up * 0.5 + 0.5, 0.0, 1.0);
  vec3 col = mix(horizon, top, pow(t, 1.4));

  // sun disc + glow
  float sun_angle = max(dot(dir, normalize(u_sun_dir)), 0.0);
  float disc = pow(sun_angle, 1200.0);
  float glow = pow(sun_angle, 12.0) * 0.35;
  col += vec3(1.0, 0.9, 0.72) * (disc * 2.0 + glow);

  // ground below horizon
  col = mix(ground, col, smoothstep(-0.02, 0.05, up));

  // horizon haze
  col = mix(col, horizon, exp(-abs(up) * 6.0) * 0.25);

  frag_color = vec4(col, 1.0);
}
