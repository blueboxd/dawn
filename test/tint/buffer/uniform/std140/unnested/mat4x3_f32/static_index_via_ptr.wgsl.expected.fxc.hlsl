cbuffer cbuffer_m : register(b0, space0) {
  uint4 m[4];
};
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

float4x3 tint_symbol(uint4 buffer[4], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz), asfloat(buffer[scalar_offset_2 / 4].xyz), asfloat(buffer[scalar_offset_3 / 4].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  const float4x3 l_m = tint_symbol(m, 0u);
  const float3 l_m_1 = asfloat(m[1].xyz);
  return;
}
