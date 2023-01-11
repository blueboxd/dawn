float4 tint_atanh(float4 x) {
  return (log(((1.0f + x) / (1.0f - x))) * 0.5f);
}

void atanh_f3e01b() {
  float4 arg_0 = (0.5f).xxxx;
  float4 res = tint_atanh(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atanh_f3e01b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atanh_f3e01b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atanh_f3e01b();
  return;
}
