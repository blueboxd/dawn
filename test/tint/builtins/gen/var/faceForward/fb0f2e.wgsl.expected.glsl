#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void faceForward_fb0f2e() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  f16vec2 arg_1 = f16vec2(1.0hf);
  f16vec2 arg_2 = f16vec2(1.0hf);
  f16vec2 res = faceforward(arg_0, arg_1, arg_2);
}

vec4 vertex_main() {
  faceForward_fb0f2e();
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

void faceForward_fb0f2e() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  f16vec2 arg_1 = f16vec2(1.0hf);
  f16vec2 arg_2 = f16vec2(1.0hf);
  f16vec2 res = faceforward(arg_0, arg_1, arg_2);
}

void fragment_main() {
  faceForward_fb0f2e();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void faceForward_fb0f2e() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  f16vec2 arg_1 = f16vec2(1.0hf);
  f16vec2 arg_2 = f16vec2(1.0hf);
  f16vec2 res = faceforward(arg_0, arg_1, arg_2);
}

void compute_main() {
  faceForward_fb0f2e();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
