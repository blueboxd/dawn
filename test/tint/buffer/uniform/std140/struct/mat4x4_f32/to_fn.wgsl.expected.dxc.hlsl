struct S {
  int before;
  float4x4 m;
  int after;
};

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[48];
};

void a(S a_1[4]) {
}

void b(S s) {
}

void c(float4x4 m) {
}

void d(float4 v) {
}

void e(float f_1) {
}

float4x4 tint_symbol_3(uint4 buffer[48], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]), asfloat(buffer[scalar_offset_3 / 4]));
}

S tint_symbol_1(uint4 buffer[48], uint offset) {
  const uint scalar_offset_4 = ((offset + 0u)) / 4;
  const uint scalar_offset_5 = ((offset + 128u)) / 4;
  const S tint_symbol_5 = {asint(buffer[scalar_offset_4 / 4][scalar_offset_4 % 4]), tint_symbol_3(buffer, (offset + 16u)), asint(buffer[scalar_offset_5 / 4][scalar_offset_5 % 4])};
  return tint_symbol_5;
}

typedef S tint_symbol_ret[4];
tint_symbol_ret tint_symbol(uint4 buffer[48], uint offset) {
  S arr[4] = (S[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = tint_symbol_1(buffer, (offset + (i * 192u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  a(tint_symbol(u, 0u));
  b(tint_symbol_1(u, 384u));
  c(tint_symbol_3(u, 400u));
  d(asfloat(u[2]).ywxz);
  e(asfloat(u[2]).ywxz.x);
  return;
}
