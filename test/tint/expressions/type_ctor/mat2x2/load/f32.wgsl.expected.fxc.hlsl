RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, float2x2 value) {
  tint_symbol.Store2((offset + 0u), asuint(value[0u]));
  tint_symbol.Store2((offset + 8u), asuint(value[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 m = float2x2((0.0f).xx, (0.0f).xx);
  tint_symbol_store(0u, float2x2(m));
  return;
}
