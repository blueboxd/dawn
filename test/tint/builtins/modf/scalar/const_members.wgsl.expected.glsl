#version 310 es

struct modf_result_f32 {
  float fract;
  float whole;
};


void tint_symbol() {
  modf_result_f32 tint_symbol_3 = modf_result_f32(0.25f, 1.0f);
  float tint_symbol_2 = tint_symbol_3.fract;
  modf_result_f32 tint_symbol_4 = modf_result_f32(0.25f, 1.0f);
  float whole = tint_symbol_4.whole;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
