SKIP: FAILED

vector<float16_t, 3> tint_radians(vector<float16_t, 3> param_0) {
  return param_0 * 0.017453292519943295474;
}

void radians_7ea4c7() {
  vector<float16_t, 3> res = tint_radians((float16_t(0.0h)).xxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  radians_7ea4c7();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  radians_7ea4c7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  radians_7ea4c7();
  return;
}
