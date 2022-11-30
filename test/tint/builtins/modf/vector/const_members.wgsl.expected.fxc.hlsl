struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};
[numthreads(1, 1, 1)]
void main() {
  const modf_result_vec2_f32 tint_symbol_1 = {float2(0.25f, 0.75f), float2(1.0f, 3.0f)};
  const float2 fract = tint_symbol_1.fract;
  const modf_result_vec2_f32 tint_symbol_2 = {float2(0.25f, 0.75f), float2(1.0f, 3.0f)};
  const float2 whole = tint_symbol_2.whole;
  return;
}
