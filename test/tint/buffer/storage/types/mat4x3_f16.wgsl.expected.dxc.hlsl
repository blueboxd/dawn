ByteAddressBuffer tint_symbol : register(t0, space0);
RWByteAddressBuffer tint_symbol_1 : register(u1, space0);

void tint_symbol_2(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 4, 3> value) {
  buffer.Store<vector<float16_t, 3> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 3> >((offset + 8u), value[1u]);
  buffer.Store<vector<float16_t, 3> >((offset + 16u), value[2u]);
  buffer.Store<vector<float16_t, 3> >((offset + 24u), value[3u]);
}

matrix<float16_t, 4, 3> tint_symbol_4(ByteAddressBuffer buffer, uint offset) {
  return matrix<float16_t, 4, 3>(buffer.Load<vector<float16_t, 3> >((offset + 0u)), buffer.Load<vector<float16_t, 3> >((offset + 8u)), buffer.Load<vector<float16_t, 3> >((offset + 16u)), buffer.Load<vector<float16_t, 3> >((offset + 24u)));
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_2(tint_symbol_1, 0u, tint_symbol_4(tint_symbol, 0u));
  return;
}
