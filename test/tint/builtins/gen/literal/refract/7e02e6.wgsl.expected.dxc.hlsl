RWByteAddressBuffer prevent_dce : register(u0, space2);

void refract_7e02e6() {
  float4 res = (-7.0f).xxxx;
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  refract_7e02e6();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  refract_7e02e6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  refract_7e02e6();
  return;
}
