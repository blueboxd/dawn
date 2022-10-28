#version 310 es

layout(r32f) uniform highp writeonly image2D arg_0;
void textureStore_1e20f2() {
  uvec2 arg_1 = uvec2(0u);
  vec4 arg_2 = vec4(0.0f);
  imageStore(arg_0, ivec2(arg_1), arg_2);
}

vec4 vertex_main() {
  textureStore_1e20f2();
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

layout(r32f) uniform highp writeonly image2D arg_0;
void textureStore_1e20f2() {
  uvec2 arg_1 = uvec2(0u);
  vec4 arg_2 = vec4(0.0f);
  imageStore(arg_0, ivec2(arg_1), arg_2);
}

void fragment_main() {
  textureStore_1e20f2();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(r32f) uniform highp writeonly image2D arg_0;
void textureStore_1e20f2() {
  uvec2 arg_1 = uvec2(0u);
  vec4 arg_2 = vec4(0.0f);
  imageStore(arg_0, ivec2(arg_1), arg_2);
}

void compute_main() {
  textureStore_1e20f2();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
