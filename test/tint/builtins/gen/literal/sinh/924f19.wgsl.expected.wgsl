enable f16;

fn sinh_924f19() {
  var res : vec2<f16> = sinh(vec2<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_924f19();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_924f19();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_924f19();
}
