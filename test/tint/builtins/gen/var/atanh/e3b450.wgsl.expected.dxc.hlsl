vector<float16_t, 4> tint_atanh(vector<float16_t, 4> x) {
  return (log(((float16_t(1.0h) + x) / (float16_t(1.0h) - x))) * float16_t(0.5h));
}

void atanh_e3b450() {
  vector<float16_t, 4> arg_0 = (float16_t(0.5h)).xxxx;
  vector<float16_t, 4> res = tint_atanh(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atanh_e3b450();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atanh_e3b450();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atanh_e3b450();
  return;
}
