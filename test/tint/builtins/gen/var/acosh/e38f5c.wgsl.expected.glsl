#version 310 es

vec3 tint_acosh(vec3 x) {
  return mix(acosh(x), vec3(0.0f), lessThan(x, vec3(1.0f)));
}

void acosh_e38f5c() {
  vec3 arg_0 = vec3(2.0f);
  vec3 res = tint_acosh(arg_0);
}

vec4 vertex_main() {
  acosh_e38f5c();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

vec3 tint_acosh(vec3 x) {
  return mix(acosh(x), vec3(0.0f), lessThan(x, vec3(1.0f)));
}

void acosh_e38f5c() {
  vec3 arg_0 = vec3(2.0f);
  vec3 res = tint_acosh(arg_0);
}

void fragment_main() {
  acosh_e38f5c();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

vec3 tint_acosh(vec3 x) {
  return mix(acosh(x), vec3(0.0f), lessThan(x, vec3(1.0f)));
}

void acosh_e38f5c() {
  vec3 arg_0 = vec3(2.0f);
  vec3 res = tint_acosh(arg_0);
}

void compute_main() {
  acosh_e38f5c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
