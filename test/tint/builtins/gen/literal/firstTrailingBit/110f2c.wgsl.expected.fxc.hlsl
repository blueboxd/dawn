void firstTrailingBit_110f2c() {
  uint4 res = (0u).xxxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  firstTrailingBit_110f2c();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  firstTrailingBit_110f2c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  firstTrailingBit_110f2c();
  return;
}
