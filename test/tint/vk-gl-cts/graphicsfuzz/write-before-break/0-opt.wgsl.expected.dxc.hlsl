SKIP: FAILED

void set_scalar_float4x3(inout float4x3 mat, int col, int row, float val) {
  switch (col) {
    case 0:
      mat[0] = (row.xxx == int3(0, 1, 2)) ? val.xxx : mat[0];
      break;
    case 1:
      mat[1] = (row.xxx == int3(0, 1, 2)) ? val.xxx : mat[1];
      break;
    case 2:
      mat[2] = (row.xxx == int3(0, 1, 2)) ? val.xxx : mat[2];
      break;
    case 3:
      mat[3] = (row.xxx == int3(0, 1, 2)) ? val.xxx : mat[3];
      break;
  }
}

cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int idx = 0;
  float4x3 m43 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int ll_1 = 0;
  int GLF_live6rows = 0;
  int z = 0;
  int ll_2 = 0;
  int ctr = 0;
  float4x3 tempm43 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int ll_3 = 0;
  int c = 0;
  int d = 0;
  float GLF_live6sums[9] = (float[9])0;
  idx = 0;
  m43 = float4x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), (0.0f).xxx);
  ll_1 = 0;
  GLF_live6rows = 2;
  while (true) {
    const int x_18 = ll_1;
    const int x_19 = asint(x_9[0].x);
    if ((x_18 >= x_19)) {
      x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
      break;
    }
    const int x_20 = ll_1;
    ll_1 = (x_20 + 1);
    const int x_22 = asint(x_9[0].x);
    z = x_22;
    ll_2 = 0;
    ctr = 0;
    while (true) {
      const int x_23 = ctr;
      if ((x_23 < 1)) {
      } else {
        break;
      }
      const int x_24 = ll_2;
      const int x_25 = asint(x_9[0].x);
      if ((x_24 >= x_25)) {
        break;
      }
      const int x_26 = ll_2;
      ll_2 = (x_26 + 1);
      const float4x3 x_98 = m43;
      tempm43 = x_98;
      ll_3 = 0;
      c = 0;
      while (true) {
        const int x_28 = z;
        if ((1 < x_28)) {
        } else {
          break;
        }
        d = 0;
        const int x_29 = c;
        const int x_30 = c;
        const int x_31 = c;
        const int x_32 = d;
        const int x_33 = d;
        const int x_34 = d;
        set_scalar_float4x3(tempm43, (((x_29 >= 0) & (x_30 < 4)) ? x_31 : 0), (((x_32 >= 0) & (x_33 < 3)) ? x_34 : 0), 1.0f);
        {
          const int x_35 = c;
          c = (x_35 + 1);
        }
      }
      const int x_37 = idx;
      const int x_38 = idx;
      const int x_39 = idx;
      const int x_117 = (((x_37 >= 0) & (x_38 < 9)) ? x_39 : 0);
      const int x_40 = ctr;
      const float x_119 = m43[x_40].y;
      const float x_121 = GLF_live6sums[x_117];
      GLF_live6sums[x_117] = (x_121 + x_119);
      {
        const int x_41 = ctr;
        ctr = (x_41 + 1);
      }
    }
    const int x_43 = idx;
    idx = (x_43 + 1);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
DXC validation failure:
warning: DXIL signing library (dxil.dll,libdxil.so) not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
shader.hlsl:121: error: Loop must have break.
Validation failed.



