cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[2];
};
static float2x4 p = float2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

float2x4 tint_symbol(uint4 buffer[2], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]));
}

[numthreads(1, 1, 1)]
void f() {
  p = tint_symbol(u, 0u);
  p[1] = asfloat(u[0]);
  p[1] = asfloat(u[0]).ywxz;
  p[0][1] = asfloat(u[1].x);
  return;
}
