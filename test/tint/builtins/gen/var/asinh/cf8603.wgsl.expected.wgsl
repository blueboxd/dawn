fn asinh_cf8603() {
  const arg_0 = vec4(1);
  var res = asinh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_cf8603();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_cf8603();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_cf8603();
}
