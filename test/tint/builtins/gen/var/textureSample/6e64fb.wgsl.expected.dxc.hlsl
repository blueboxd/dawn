Texture1D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSample_6e64fb() {
  float arg_2 = 1.0f;
  float4 res = arg_0.Sample(arg_1, arg_2);
  prevent_dce.Store4(0u, asuint(res));
}

void fragment_main() {
  textureSample_6e64fb();
  return;
}
