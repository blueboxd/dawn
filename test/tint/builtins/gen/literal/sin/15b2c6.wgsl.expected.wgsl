fn sin_15b2c6() {
  var res = sin(vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_15b2c6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_15b2c6();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_15b2c6();
}
