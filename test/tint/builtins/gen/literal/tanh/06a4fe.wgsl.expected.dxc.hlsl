void tanh_06a4fe() {
  vector<float16_t, 3> res = (float16_t(0.761230469h)).xxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  tanh_06a4fe();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  tanh_06a4fe();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_06a4fe();
  return;
}
