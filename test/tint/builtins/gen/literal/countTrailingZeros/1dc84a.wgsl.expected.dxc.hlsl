void countTrailingZeros_1dc84a() {
  int4 res = (0).xxxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  countTrailingZeros_1dc84a();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  countTrailingZeros_1dc84a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countTrailingZeros_1dc84a();
  return;
}
