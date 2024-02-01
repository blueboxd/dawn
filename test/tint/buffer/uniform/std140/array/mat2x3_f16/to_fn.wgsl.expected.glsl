#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat2x3_f16 {
  f16vec3 col0;
  f16vec3 col1;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat2x3_f16 inner[4];
} u;

layout(binding = 1, std430) buffer s_block_ssbo {
  float16_t inner;
} s;

float16_t a(f16mat2x3 a_1[4]) {
  return a_1[0][0].x;
}

float16_t b(f16mat2x3 m) {
  return m[0].x;
}

float16_t c(f16vec3 v) {
  return v.x;
}

float16_t d(float16_t f_1) {
  return f_1;
}

f16mat2x3 conv_mat2x3_f16(mat2x3_f16 val) {
  return f16mat2x3(val.col0, val.col1);
}

f16mat2x3[4] conv_arr4_mat2x3_f16(mat2x3_f16 val[4]) {
  f16mat2x3 arr[4] = f16mat2x3[4](f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat2x3_f16(val[i]);
    }
  }
  return arr;
}

void f() {
  float16_t tint_symbol = a(conv_arr4_mat2x3_f16(u.inner));
  float16_t tint_symbol_1 = b(conv_mat2x3_f16(u.inner[1u]));
  float16_t tint_symbol_2 = c(u.inner[1u].col0.zxy);
  float16_t tint_symbol_3 = d(u.inner[1u].col0.zxy[0u]);
  s.inner = (((tint_symbol + tint_symbol_1) + tint_symbol_2) + tint_symbol_3);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
