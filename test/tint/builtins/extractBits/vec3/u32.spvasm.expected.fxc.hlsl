uint3 tint_extract_bits(uint3 v, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint shl = (32u - e);
  const uint shr = (shl + s);
  const uint3 shl_result = ((shl < 32u) ? (v << uint3((shl).xxx)) : (0u).xxx);
  return ((shr < 32u) ? (shl_result >> uint3((shr).xxx)) : ((shl_result >> (31u).xxx) >> (1u).xxx));
}

void f_1() {
  uint3 v = (0u).xxx;
  uint offset_1 = 0u;
  uint count = 0u;
  const uint3 x_16 = v;
  const uint x_17 = offset_1;
  const uint x_18 = count;
  const uint3 x_14 = tint_extract_bits(x_16, x_17, x_18);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
