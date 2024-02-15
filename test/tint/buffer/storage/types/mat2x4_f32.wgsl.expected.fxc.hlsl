ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);

void tint_symbol_1_store(uint offset, float2x4 value) {
  tint_symbol_1.Store4((offset + 0u), asuint(value[0u]));
  tint_symbol_1.Store4((offset + 16u), asuint(value[1u]));
}

float2x4 tint_symbol_load(uint offset) {
  return float2x4(asfloat(tint_symbol.Load4((offset + 0u))), asfloat(tint_symbol.Load4((offset + 16u))));
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_1_store(0u, tint_symbol_load(0u));
  return;
}