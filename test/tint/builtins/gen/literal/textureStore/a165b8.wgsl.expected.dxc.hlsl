RWTexture2D<float4> arg_0 : register(u0, space1);

void textureStore_a165b8() {
  arg_0[(1u).xx] = (1.0f).xxxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureStore_a165b8();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureStore_a165b8();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_a165b8();
  return;
}
