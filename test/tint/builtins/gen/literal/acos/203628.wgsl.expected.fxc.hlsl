SKIP: FAILED

void acos_203628() {
  vector<float16_t, 4> res = acos((float16_t(0.0h)).xxxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  acos_203628();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  acos_203628();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acos_203628();
  return;
}
