#version 310 es

void f() {
  ivec3 a = ivec3(1073757184, -1006616064, -998242304);
  ivec3 b = a;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
