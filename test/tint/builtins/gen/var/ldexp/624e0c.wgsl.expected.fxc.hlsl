SKIP: FAILED

void ldexp_624e0c() {
  float16_t arg_0 = float16_t(1.0h);
  int arg_1 = 1;
  float16_t res = ldexp(arg_0, arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  ldexp_624e0c();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  ldexp_624e0c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_624e0c();
  return;
}
FXC validation failure:
C:\src\dawn\test\tint\Shader@0x0000026F55AA1260(2,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\test\tint\Shader@0x0000026F55AA1260(2,13-17): error X3000: unrecognized identifier 'arg_0'

