fn abs_2f861b() {
  var res = abs(vec3(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_2f861b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_2f861b();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_2f861b();
}
