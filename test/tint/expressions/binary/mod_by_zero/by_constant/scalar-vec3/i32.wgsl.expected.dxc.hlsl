int3 tint_mod(int lhs, int3 rhs) {
  const int3 l = int3((lhs).xxx);
  return (l % (((rhs == (0).xxx) | ((l == (-2147483648).xxx) & (rhs == (-1).xxx))) ? (1).xxx : rhs));
}

[numthreads(1, 1, 1)]
void f() {
  const int a = 4;
  const int3 b = int3(0, 2, 0);
  const int3 r = tint_mod(a, b);
  return;
}
