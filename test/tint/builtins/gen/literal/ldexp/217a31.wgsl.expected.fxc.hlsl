SKIP: FAILED

void ldexp_217a31() {
  vector<float16_t, 2> res = (float16_t(2.0h)).xx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  ldexp_217a31();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  ldexp_217a31();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_217a31();
  return;
}
FXC validation failure:
C:\src\dawn\test\tint\Shader@0x00000132D8550FB0(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

