#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uvec3 u = uvec3(1u);
void f() {
  f16vec3 v = f16vec3(u);
}

