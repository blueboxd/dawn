#version 310 es

uniform highp sampler2DArray arg_0_1;
void textureLoad_04b911() {
  uvec2 arg_1 = uvec2(0u);
  int arg_2 = 1;
  uint arg_3 = 1u;
  float res = texelFetch(arg_0_1, ivec3(uvec3(arg_1, uint(arg_2))), int(arg_3)).x;
}

vec4 vertex_main() {
  textureLoad_04b911();
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

uniform highp sampler2DArray arg_0_1;
void textureLoad_04b911() {
  uvec2 arg_1 = uvec2(0u);
  int arg_2 = 1;
  uint arg_3 = 1u;
  float res = texelFetch(arg_0_1, ivec3(uvec3(arg_1, uint(arg_2))), int(arg_3)).x;
}

void fragment_main() {
  textureLoad_04b911();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp sampler2DArray arg_0_1;
void textureLoad_04b911() {
  uvec2 arg_1 = uvec2(0u);
  int arg_2 = 1;
  uint arg_3 = 1u;
  float res = texelFetch(arg_0_1, ivec3(uvec3(arg_1, uint(arg_2))), int(arg_3)).x;
}

void compute_main() {
  textureLoad_04b911();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
