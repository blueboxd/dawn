cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[2];
};

void a(float2x3 m) {
}

void b(float3 v) {
}

void c(float f_1) {
}

float2x3 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x3(asfloat(u[scalar_offset / 4].xyz), asfloat(u[scalar_offset_1 / 4].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  a(u_load(0u));
  b(asfloat(u[1].xyz));
  b(asfloat(u[1].xyz).zxy);
  c(asfloat(u[1].x));
  c(asfloat(u[1].xyz).zxy.x);
  return;
}
