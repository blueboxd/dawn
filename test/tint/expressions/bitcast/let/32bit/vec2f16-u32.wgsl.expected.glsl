#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

uint tint_bitcast_from_f16(f16vec2 src) {
  uint r = packFloat2x16(src);
  return uint(r);
}

void f() {
  f16vec2 a = f16vec2(1.0hf, 2.0hf);
  uint b = tint_bitcast_from_f16(a);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
