SKIP: FAILED

void sinh_69cce2() {
  float16_t res = sinh(float16_t(0.0h));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  sinh_69cce2();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  sinh_69cce2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sinh_69cce2();
  return;
}
