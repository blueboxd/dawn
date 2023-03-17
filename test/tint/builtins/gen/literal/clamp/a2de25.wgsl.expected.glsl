#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void clamp_a2de25() {
  uint res = 1u;
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  clamp_a2de25();
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
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void clamp_a2de25() {
  uint res = 1u;
  prevent_dce.inner = res;
}

void fragment_main() {
  clamp_a2de25();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void clamp_a2de25() {
  uint res = 1u;
  prevent_dce.inner = res;
}

void compute_main() {
  clamp_a2de25();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
