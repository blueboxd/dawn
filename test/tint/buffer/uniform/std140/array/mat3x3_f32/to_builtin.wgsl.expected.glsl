#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat3 inner[4];
} u;

layout(binding = 1, std430) buffer s_block_ssbo {
  float inner;
} s;

void f() {
  mat3 t = transpose(u.inner[2]);
  float l = length(u.inner[0][1].zxy);
  float a = abs(u.inner[0][1].zxy.x);
  s.inner = ((t[0].x + float(l)) + float(a));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
