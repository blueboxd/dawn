void insertBits_428b0b() {
  int3 res = (3).xxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  insertBits_428b0b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  insertBits_428b0b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  insertBits_428b0b();
  return;
}
