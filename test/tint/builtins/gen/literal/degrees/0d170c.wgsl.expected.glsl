#version 310 es

void degrees_0d170c() {
  vec4 res = vec4(57.2957763671875f);
}

vec4 vertex_main() {
  degrees_0d170c();
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

void degrees_0d170c() {
  vec4 res = vec4(57.2957763671875f);
}

void fragment_main() {
  degrees_0d170c();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void degrees_0d170c() {
  vec4 res = vec4(57.2957763671875f);
}

void compute_main() {
  degrees_0d170c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
