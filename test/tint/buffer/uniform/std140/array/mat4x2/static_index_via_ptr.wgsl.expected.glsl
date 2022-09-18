#version 310 es

struct mat4x2_f32 {
  vec2 col0;
  vec2 col1;
  vec2 col2;
  vec2 col3;
};

layout(binding = 0, std140) uniform a_block_std140_ubo {
  mat4x2_f32 inner[4];
} a;

mat4x2 conv_mat4x2_f32(mat4x2_f32 val) {
  return mat4x2(val.col0, val.col1, val.col2, val.col3);
}

mat4x2[4] conv_arr4_mat4x2_f32(mat4x2_f32 val[4]) {
  mat4x2 arr[4] = mat4x2[4](mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat4x2_f32(val[i]);
    }
  }
  return arr;
}

void f() {
  mat4x2 p_a[4] = conv_arr4_mat4x2_f32(a.inner);
  mat4x2 p_a_2 = conv_mat4x2_f32(a.inner[2u]);
  vec2 p_a_2_1 = a.inner[2u].col1;
  mat4x2 l_a[4] = conv_arr4_mat4x2_f32(a.inner);
  mat4x2 l_a_i = conv_mat4x2_f32(a.inner[2u]);
  vec2 l_a_i_i = a.inner[2u].col1;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
