static float2 arg_1;

void modf_9c6a91() {
  float2 res = modf(float2(0.0f, 0.0f), arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  modf_9c6a91();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  modf_9c6a91();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_9c6a91();
  return;
}
