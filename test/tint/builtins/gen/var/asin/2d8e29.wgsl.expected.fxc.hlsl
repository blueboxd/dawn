SKIP: FAILED

void asin_2d8e29() {
  vector<float16_t, 3> arg_0 = (float16_t(0.479248047h)).xxx;
  vector<float16_t, 3> res = asin(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  asin_2d8e29();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  asin_2d8e29();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asin_2d8e29();
  return;
}
