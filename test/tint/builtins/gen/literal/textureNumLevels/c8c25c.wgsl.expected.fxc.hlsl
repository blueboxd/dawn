TextureCube arg_0 : register(t0, space1);

void textureNumLevels_c8c25c() {
  int3 tint_tmp;
  arg_0.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  uint res = tint_tmp.z;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureNumLevels_c8c25c();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureNumLevels_c8c25c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumLevels_c8c25c();
  return;
}
