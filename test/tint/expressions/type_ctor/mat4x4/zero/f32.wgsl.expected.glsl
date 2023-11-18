#version 310 es

mat4 m = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  mat4 inner;
} tint_symbol;

void f() {
  tint_symbol.inner = m;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
