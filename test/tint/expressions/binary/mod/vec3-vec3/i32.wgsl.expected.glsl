#version 310 es

ivec3 tint_mod(ivec3 lhs, ivec3 rhs) {
  return (lhs % mix(rhs, ivec3(1), bvec3(uvec3(equal(rhs, ivec3(0))) | uvec3(bvec3(uvec3(equal(lhs, ivec3(-2147483648))) & uvec3(equal(rhs, ivec3(-1))))))));
}

void f() {
  ivec3 a = ivec3(1, 2, 3);
  ivec3 b = ivec3(4, 5, 6);
  ivec3 r = tint_mod(a, b);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
