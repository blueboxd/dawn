#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void f() {
  f16mat3x2 m = f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf));
  f16mat3x2 m_1 = f16mat3x2(m);
}

