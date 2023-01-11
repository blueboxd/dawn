#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_acosh(float16_t x) {
  return ((x < 1.0hf) ? 0.0hf : acosh(x));
}

void acosh_a37dfe() {
  float16_t arg_0 = 2.0hf;
  float16_t res = tint_acosh(arg_0);
}

vec4 vertex_main() {
  acosh_a37dfe();
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
#extension GL_AMD_gpu_shader_half_float : require
precision mediump float;

float16_t tint_acosh(float16_t x) {
  return ((x < 1.0hf) ? 0.0hf : acosh(x));
}

void acosh_a37dfe() {
  float16_t arg_0 = 2.0hf;
  float16_t res = tint_acosh(arg_0);
}

void fragment_main() {
  acosh_a37dfe();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_acosh(float16_t x) {
  return ((x < 1.0hf) ? 0.0hf : acosh(x));
}

void acosh_a37dfe() {
  float16_t arg_0 = 2.0hf;
  float16_t res = tint_acosh(arg_0);
}

void compute_main() {
  acosh_a37dfe();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
