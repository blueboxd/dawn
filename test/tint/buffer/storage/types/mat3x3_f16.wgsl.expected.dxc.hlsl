ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);

void tint_symbol_1_store(uint offset, matrix<float16_t, 3, 3> value) {
  tint_symbol_1.Store<vector<float16_t, 3> >((offset + 0u), value[0u]);
  tint_symbol_1.Store<vector<float16_t, 3> >((offset + 8u), value[1u]);
  tint_symbol_1.Store<vector<float16_t, 3> >((offset + 16u), value[2u]);
}

matrix<float16_t, 3, 3> tint_symbol_load(uint offset) {
  return matrix<float16_t, 3, 3>(tint_symbol.Load<vector<float16_t, 3> >((offset + 0u)), tint_symbol.Load<vector<float16_t, 3> >((offset + 8u)), tint_symbol.Load<vector<float16_t, 3> >((offset + 16u)));
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_1_store(0u, tint_symbol_load(0u));
  return;
}
