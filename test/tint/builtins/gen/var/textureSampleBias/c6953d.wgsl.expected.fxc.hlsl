TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleBias_c6953d() {
  float3 arg_2 = (0.0f).xxx;
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  float4 res = arg_0.SampleBias(arg_1, float4(arg_2, float(arg_3)), arg_4);
}

void fragment_main() {
  textureSampleBias_c6953d();
  return;
}
