#version 310 es

void f() {
  ivec4 a = ivec4(1073757184, -1006616064, -998242304, 987654321);
  uvec4 b = uvec4(a);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
