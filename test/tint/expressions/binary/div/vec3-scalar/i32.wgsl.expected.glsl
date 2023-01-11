#version 310 es

ivec3 tint_div(ivec3 lhs, int rhs) {
  ivec3 r = ivec3(rhs);
  return (lhs / mix(r, ivec3(1), bvec3(uvec3(equal(r, ivec3(0))) | uvec3(bvec3(uvec3(equal(lhs, ivec3(-2147483648))) & uvec3(equal(r, ivec3(-1))))))));
}

void f() {
  ivec3 a = ivec3(1, 2, 3);
  int b = 4;
  ivec3 r = tint_div(a, b);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
