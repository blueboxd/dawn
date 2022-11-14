#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec3 tint_degrees(f16vec3 param_0) {
  return param_0 * 57.295779513082322865hf;
}


void degrees_dfe8f4() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  f16vec3 res = tint_degrees(arg_0);
}

vec4 vertex_main() {
  degrees_dfe8f4();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision mediump float;

f16vec3 tint_degrees(f16vec3 param_0) {
  return param_0 * 57.295779513082322865hf;
}


void degrees_dfe8f4() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  f16vec3 res = tint_degrees(arg_0);
}

void fragment_main() {
  degrees_dfe8f4();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec3 tint_degrees(f16vec3 param_0) {
  return param_0 * 57.295779513082322865hf;
}


void degrees_dfe8f4() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  f16vec3 res = tint_degrees(arg_0);
}

void compute_main() {
  degrees_dfe8f4();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
