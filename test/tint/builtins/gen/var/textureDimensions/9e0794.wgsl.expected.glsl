#version 310 es

uniform highp usampler2DArray arg_0_1;
void textureDimensions_9e0794() {
  int arg_1 = 1;
  uvec2 res = uvec2(textureSize(arg_0_1, arg_1).xy);
}

vec4 vertex_main() {
  textureDimensions_9e0794();
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

uniform highp usampler2DArray arg_0_1;
void textureDimensions_9e0794() {
  int arg_1 = 1;
  uvec2 res = uvec2(textureSize(arg_0_1, arg_1).xy);
}

void fragment_main() {
  textureDimensions_9e0794();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp usampler2DArray arg_0_1;
void textureDimensions_9e0794() {
  int arg_1 = 1;
  uvec2 res = uvec2(textureSize(arg_0_1, arg_1).xy);
}

void compute_main() {
  textureDimensions_9e0794();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
