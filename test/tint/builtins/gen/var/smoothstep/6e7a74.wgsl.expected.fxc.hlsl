SKIP: FAILED

void smoothstep_6e7a74() {
  vector<float16_t, 3> arg_0 = (float16_t(0.0h)).xxx;
  vector<float16_t, 3> arg_1 = (float16_t(0.0h)).xxx;
  vector<float16_t, 3> arg_2 = (float16_t(0.0h)).xxx;
  vector<float16_t, 3> res = smoothstep(arg_0, arg_1, arg_2);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  smoothstep_6e7a74();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  smoothstep_6e7a74();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothstep_6e7a74();
  return;
}
