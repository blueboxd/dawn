RWByteAddressBuffer prevent_dce : register(u0, space2);

void step_334303() {
  float3 arg_0 = (1.0f).xxx;
  float3 arg_1 = (1.0f).xxx;
  float3 res = step(arg_0, arg_1);
  prevent_dce.Store3(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  step_334303();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  step_334303();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  step_334303();
  return;
}
