#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void f() {
  int i = 1;
  vec2 a = mat4x2(vec2(0.0f), vec2(0.0f), vec2(4.0f, 0.0f), vec2(0.0f))[i];
  int b = ivec2(0, 1)[i];
}

