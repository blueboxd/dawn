#version 310 es

void mix_6f8adc() {
  vec2 res = vec2(1.0f);
}

vec4 vertex_main() {
  mix_6f8adc();
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
precision mediump float;

void mix_6f8adc() {
  vec2 res = vec2(1.0f);
}

void fragment_main() {
  mix_6f8adc();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void mix_6f8adc() {
  vec2 res = vec2(1.0f);
}

void compute_main() {
  mix_6f8adc();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
