#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void tanh_5b19af() {
  float16_t arg_0 = 1.0hf;
  float16_t res = tanh(arg_0);
}

vec4 vertex_main() {
  tanh_5b19af();
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

void tanh_5b19af() {
  float16_t arg_0 = 1.0hf;
  float16_t res = tanh(arg_0);
}

void fragment_main() {
  tanh_5b19af();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void tanh_5b19af() {
  float16_t arg_0 = 1.0hf;
  float16_t res = tanh(arg_0);
}

void compute_main() {
  tanh_5b19af();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
