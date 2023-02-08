struct frexp_result_vec2_f16 {
  vector<float16_t, 2> fract;
  int2 exp;
};
frexp_result_vec2_f16 tint_frexp(vector<float16_t, 2> param_0) {
  vector<float16_t, 2> exp;
  vector<float16_t, 2> fract = frexp(param_0, exp);
  frexp_result_vec2_f16 result = {fract, int2(exp)};
  return result;
}

void frexp_5f47bf() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  frexp_result_vec2_f16 res = tint_frexp(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_5f47bf();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_5f47bf();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_5f47bf();
  return;
}
