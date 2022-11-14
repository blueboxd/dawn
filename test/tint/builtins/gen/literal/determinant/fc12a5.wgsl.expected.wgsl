enable f16;

fn determinant_fc12a5() {
  var res : f16 = determinant(mat2x2<f16>(1.0h, 1.0h, 1.0h, 1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  determinant_fc12a5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  determinant_fc12a5();
}

@compute @workgroup_size(1)
fn compute_main() {
  determinant_fc12a5();
}
