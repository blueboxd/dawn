SKIP: FAILED

void transpose_06794e() {
  matrix<float16_t, 3, 3> arg_0 = matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
  matrix<float16_t, 3, 3> res = transpose(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  transpose_06794e();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  transpose_06794e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_06794e();
  return;
}
