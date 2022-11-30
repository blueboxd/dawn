#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 1, std430) buffer SB_RO_ssbo {
  float16_t arg_0[];
} sb_ro;

void arrayLength_8421b9() {
  uint res = uint(sb_ro.arg_0.length());
}

vec4 vertex_main() {
  arrayLength_8421b9();
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

layout(binding = 1, std430) buffer SB_RO_ssbo {
  float16_t arg_0[];
} sb_ro;

void arrayLength_8421b9() {
  uint res = uint(sb_ro.arg_0.length());
}

void fragment_main() {
  arrayLength_8421b9();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 1, std430) buffer SB_RO_ssbo {
  float16_t arg_0[];
} sb_ro;

void arrayLength_8421b9() {
  uint res = uint(sb_ro.arg_0.length());
}

void compute_main() {
  arrayLength_8421b9();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
