fn inverseSqrt_4ca6d6() {
  var res = inverseSqrt(1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  inverseSqrt_4ca6d6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  inverseSqrt_4ca6d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_4ca6d6();
}
