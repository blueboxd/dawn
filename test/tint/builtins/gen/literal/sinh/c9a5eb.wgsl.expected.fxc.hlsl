void sinh_c9a5eb() {
  float3 res = (1.17520117759704589844f).xxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  sinh_c9a5eb();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  sinh_c9a5eb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sinh_c9a5eb();
  return;
}
