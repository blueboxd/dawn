fn abs_5a8af1() {
  var res = abs(1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_5a8af1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_5a8af1();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_5a8af1();
}
