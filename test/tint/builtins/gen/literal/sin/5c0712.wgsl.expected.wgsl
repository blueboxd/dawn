enable f16;

fn sin_5c0712() {
  var res : vec4<f16> = sin(vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_5c0712();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_5c0712();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_5c0712();
}
