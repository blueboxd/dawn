void abs_fd247f() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t res = abs(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  abs_fd247f();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  abs_fd247f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_fd247f();
  return;
}
