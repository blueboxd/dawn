#version 310 es

void f() {
  uvec4 a = uvec4(1073757184u, 3288351232u, 3296724992u, 987654321u);
  ivec4 b = ivec4(a);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
