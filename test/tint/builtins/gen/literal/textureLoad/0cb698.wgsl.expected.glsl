SKIP: FAILED

#version 310 es

uniform highp isampler1D arg_0_1;
void textureLoad_0cb698() {
  ivec4 res = texelFetch(arg_0_1, int(1u), int(1u));
}

vec4 vertex_main() {
  textureLoad_0cb698();
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
Error parsing GLSL shader:
ERROR: 0:3: 'isampler1D' : Reserved word. 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp isampler1D arg_0_1;
void textureLoad_0cb698() {
  ivec4 res = texelFetch(arg_0_1, int(1u), int(1u));
}

void fragment_main() {
  textureLoad_0cb698();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'isampler1D' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uniform highp isampler1D arg_0_1;
void textureLoad_0cb698() {
  ivec4 res = texelFetch(arg_0_1, int(1u), int(1u));
}

void compute_main() {
  textureLoad_0cb698();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:3: 'isampler1D' : Reserved word. 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



