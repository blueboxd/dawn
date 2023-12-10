#version 310 es

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  mat4x2 inner;
} tint_symbol;

void f() {
  tint_symbol.inner = mat4x2(vec2(0.0f, 1.0f), vec2(2.0f, 3.0f), vec2(4.0f, 5.0f), vec2(6.0f, 7.0f));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
