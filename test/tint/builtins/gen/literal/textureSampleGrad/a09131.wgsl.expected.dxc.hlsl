Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleGrad_a09131() {
  float4 res = arg_0.SampleGrad(arg_1, float3((1.0f).xx, float(1u)), (1.0f).xx, (1.0f).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureSampleGrad_a09131();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureSampleGrad_a09131();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleGrad_a09131();
  return;
}
