SKIP: FAILED

struct frexp_result_vec4_f16 {
  vector<float16_t, 4> sig;
  int4 exp;
};
frexp_result_vec4_f16 tint_frexp(vector<float16_t, 4> param_0) {
  vector<float16_t, 4> exp;
  vector<float16_t, 4> sig = frexp(param_0, exp);
  frexp_result_vec4_f16 result = {sig, int4(exp)};
  return result;
}

void frexp_3dd21e() {
  frexp_result_vec4_f16 res = tint_frexp((float16_t(0.0h)).xxxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_3dd21e();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_3dd21e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_3dd21e();
  return;
}
