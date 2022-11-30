#version 310 es

struct S {
  int before;
  uint pad;
  uint pad_1;
  uint pad_2;
  mat2x3 m;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  uint pad_6;
  int after;
  uint pad_7;
  uint pad_8;
  uint pad_9;
  uint pad_10;
  uint pad_11;
  uint pad_12;
  uint pad_13;
  uint pad_14;
  uint pad_15;
  uint pad_16;
  uint pad_17;
  uint pad_18;
  uint pad_19;
  uint pad_20;
  uint pad_21;
};

layout(binding = 0, std140) uniform u_block_ubo {
  S inner[4];
} u;

S p[4] = S[4](S(0, 0u, 0u, 0u, mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, 0u, 0u, 0u, mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, 0u, 0u, 0u, mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, 0u, 0u, 0u, mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
void f() {
  p = u.inner;
  p[1] = u.inner[2];
  p[3].m = u.inner[2].m;
  p[1].m[0] = u.inner[0].m[1].zxy;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
