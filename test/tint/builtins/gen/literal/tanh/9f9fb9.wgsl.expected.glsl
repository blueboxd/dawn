#version 310 es

void tanh_9f9fb9() {
  vec3 res = vec3(0.761594176f);
}

vec4 vertex_main() {
  tanh_9f9fb9();
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

void tanh_9f9fb9() {
  vec3 res = vec3(0.761594176f);
}

void fragment_main() {
  tanh_9f9fb9();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void tanh_9f9fb9() {
  vec3 res = vec3(0.761594176f);
}

void compute_main() {
  tanh_9f9fb9();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
