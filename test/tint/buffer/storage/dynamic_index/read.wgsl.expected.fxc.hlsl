ByteAddressBuffer sb : register(t0, space0);

struct tint_symbol_1 {
  uint idx : SV_GroupIndex;
};

float2x2 sb_load_12(uint offset) {
  return float2x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))));
}

float2x3 sb_load_13(uint offset) {
  return float2x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))));
}

float2x4 sb_load_14(uint offset) {
  return float2x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))));
}

float3x2 sb_load_15(uint offset) {
  return float3x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))));
}

float3x3 sb_load_16(uint offset) {
  return float3x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))));
}

float3x4 sb_load_17(uint offset) {
  return float3x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))));
}

float4x2 sb_load_18(uint offset) {
  return float4x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))), asfloat(sb.Load2((offset + 24u))));
}

float4x3 sb_load_19(uint offset) {
  return float4x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))), asfloat(sb.Load3((offset + 48u))));
}

float4x4 sb_load_20(uint offset) {
  return float4x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))), asfloat(sb.Load4((offset + 48u))));
}

typedef float3 sb_load_21_ret[2];
sb_load_21_ret sb_load_21(uint offset) {
  float3 arr_1[2] = (float3[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr_1[i] = asfloat(sb.Load3((offset + (i * 16u))));
    }
  }
  return arr_1;
}

void main_inner(uint idx) {
  const float scalar_f32 = asfloat(sb.Load((544u * idx)));
  const int scalar_i32 = asint(sb.Load(((544u * idx) + 4u)));
  const uint scalar_u32 = sb.Load(((544u * idx) + 8u));
  const float2 vec2_f32 = asfloat(sb.Load2(((544u * idx) + 16u)));
  const int2 vec2_i32 = asint(sb.Load2(((544u * idx) + 24u)));
  const uint2 vec2_u32 = sb.Load2(((544u * idx) + 32u));
  const float3 vec3_f32 = asfloat(sb.Load3(((544u * idx) + 48u)));
  const int3 vec3_i32 = asint(sb.Load3(((544u * idx) + 64u)));
  const uint3 vec3_u32 = sb.Load3(((544u * idx) + 80u));
  const float4 vec4_f32 = asfloat(sb.Load4(((544u * idx) + 96u)));
  const int4 vec4_i32 = asint(sb.Load4(((544u * idx) + 112u)));
  const uint4 vec4_u32 = sb.Load4(((544u * idx) + 128u));
  const float2x2 mat2x2_f32 = sb_load_12(((544u * idx) + 144u));
  const float2x3 mat2x3_f32 = sb_load_13(((544u * idx) + 160u));
  const float2x4 mat2x4_f32 = sb_load_14(((544u * idx) + 192u));
  const float3x2 mat3x2_f32 = sb_load_15(((544u * idx) + 224u));
  const float3x3 mat3x3_f32 = sb_load_16(((544u * idx) + 256u));
  const float3x4 mat3x4_f32 = sb_load_17(((544u * idx) + 304u));
  const float4x2 mat4x2_f32 = sb_load_18(((544u * idx) + 352u));
  const float4x3 mat4x3_f32 = sb_load_19(((544u * idx) + 384u));
  const float4x4 mat4x4_f32 = sb_load_20(((544u * idx) + 448u));
  const float3 arr2_vec3_f32[2] = sb_load_21(((544u * idx) + 512u));
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.idx);
  return;
}
