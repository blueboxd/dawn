enable f16;

fn acos_004aff() {
  var res : vec2<f16> = acos(vec2<f16>(0.96875h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_004aff();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_004aff();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_004aff();
}
