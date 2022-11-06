#version 310 es

uniform highp isampler2DArray arg_0_1;
void textureLoad_2363be() {
  ivec4 res = texelFetch(arg_0_1, ivec3(uvec3(uvec2(1u), uint(1))), int(1u));
}

vec4 vertex_main() {
  textureLoad_2363be();
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

uniform highp isampler2DArray arg_0_1;
void textureLoad_2363be() {
  ivec4 res = texelFetch(arg_0_1, ivec3(uvec3(uvec2(1u), uint(1))), int(1u));
}

void fragment_main() {
  textureLoad_2363be();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp isampler2DArray arg_0_1;
void textureLoad_2363be() {
  ivec4 res = texelFetch(arg_0_1, ivec3(uvec3(uvec2(1u), uint(1))), int(1u));
}

void compute_main() {
  textureLoad_2363be();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
