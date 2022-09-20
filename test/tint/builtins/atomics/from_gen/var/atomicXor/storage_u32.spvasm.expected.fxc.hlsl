RWByteAddressBuffer sb_rw : register(u0, space0);

uint tint_atomicXor(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedXor(offset, value, original_value);
  return original_value;
}


void atomicXor_54510e() {
  uint arg_1 = 0u;
  uint res = 0u;
  arg_1 = 1u;
  const uint x_18 = arg_1;
  const uint x_13 = tint_atomicXor(sb_rw, 0u, x_18);
  res = x_13;
  return;
}

void fragment_main_1() {
  atomicXor_54510e();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicXor_54510e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
