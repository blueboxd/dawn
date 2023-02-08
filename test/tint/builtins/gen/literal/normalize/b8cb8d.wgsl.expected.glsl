#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void normalize_b8cb8d() {
  f16vec4 res = f16vec4(0.5hf);
}

vec4 vertex_main() {
  normalize_b8cb8d();
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

void normalize_b8cb8d() {
  f16vec4 res = f16vec4(0.5hf);
}

void fragment_main() {
  normalize_b8cb8d();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void normalize_b8cb8d() {
  f16vec4 res = f16vec4(0.5hf);
}

void compute_main() {
  normalize_b8cb8d();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
