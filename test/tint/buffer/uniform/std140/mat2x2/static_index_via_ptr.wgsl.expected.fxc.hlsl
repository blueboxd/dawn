struct Inner {
  float2x2 m;
};
struct Outer {
  Inner a[4];
};

cbuffer cbuffer_a : register(b0, space0) {
  uint4 a[16];
};

float2x2 tint_symbol_4(uint4 buffer[16], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

Inner tint_symbol_3(uint4 buffer[16], uint offset) {
  const Inner tint_symbol_7 = {tint_symbol_4(buffer, (offset + 0u))};
  return tint_symbol_7;
}

typedef Inner tint_symbol_2_ret[4];
tint_symbol_2_ret tint_symbol_2(uint4 buffer[16], uint offset) {
  Inner arr[4] = (Inner[4])0;
  {
    [loop] for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = tint_symbol_3(buffer, (offset + (i * 16u)));
    }
  }
  return arr;
}

Outer tint_symbol_1(uint4 buffer[16], uint offset) {
  const Outer tint_symbol_8 = {tint_symbol_2(buffer, (offset + 0u))};
  return tint_symbol_8;
}

typedef Outer tint_symbol_ret[4];
tint_symbol_ret tint_symbol(uint4 buffer[16], uint offset) {
  Outer arr_1[4] = (Outer[4])0;
  {
    [loop] for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr_1[i_1] = tint_symbol_1(buffer, (offset + (i_1 * 64u)));
    }
  }
  return arr_1;
}

[numthreads(1, 1, 1)]
void f() {
  const int I = 1;
  const Outer l_a[4] = tint_symbol(a, 0u);
  const Outer l_a_3 = tint_symbol_1(a, 192u);
  const Inner l_a_3_a[4] = tint_symbol_2(a, 192u);
  const Inner l_a_3_a_2 = tint_symbol_3(a, 224u);
  const float2x2 l_a_3_a_2_m = tint_symbol_4(a, 224u);
  const float2 l_a_3_a_2_m_1 = asfloat(a[14].zw);
  const float l_a_3_a_2_m_1_0 = asfloat(a[14].z);
  return;
}
