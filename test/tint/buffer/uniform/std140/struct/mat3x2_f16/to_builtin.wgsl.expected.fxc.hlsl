SKIP: FAILED

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[32];
};

matrix<float16_t, 3, 2> tint_symbol(uint4 buffer[32], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = buffer[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = buffer[scalar_offset_1 / 4][scalar_offset_1 % 4];
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint ubo_load_2 = buffer[scalar_offset_2 / 4][scalar_offset_2 % 4];
  return matrix<float16_t, 3, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))));
}

[numthreads(1, 1, 1)]
void f() {
  const matrix<float16_t, 2, 3> t = transpose(tint_symbol(u, 260u));
  uint ubo_load_3 = u[0].z;
  const float16_t l = length(vector<float16_t, 2>(float16_t(f16tof32(ubo_load_3 & 0xFFFF)), float16_t(f16tof32(ubo_load_3 >> 16))).yx);
  uint ubo_load_4 = u[0].z;
  const float16_t a = abs(vector<float16_t, 2>(float16_t(f16tof32(ubo_load_4 & 0xFFFF)), float16_t(f16tof32(ubo_load_4 >> 16))).yx.x);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x000001CB6BBFDD80(5,8-16): error X3000: syntax error: unexpected token 'float16_t'

