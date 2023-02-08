cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[8];
};

float2x3 tint_symbol(uint4 buffer[8], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  const float3x2 t = transpose(tint_symbol(u, 64u));
  const float l = length(asfloat(u[1].xyz).zxy);
  const float a = abs(asfloat(u[1].xyz).zxy.x);
  return;
}
