float3 tint_float_mod(float lhs, float3 rhs) {
  const float3 l = float3((lhs).xxx);
  return (l - (trunc((l / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void f() {
  const float a = 4.0f;
  const float3 b = float3(1.0f, 2.0f, 3.0f);
  const float3 r = tint_float_mod(a, b);
  return;
}
