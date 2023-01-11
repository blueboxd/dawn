enable f16;

fn transpose_8c06ce() {
  var res : mat4x3<f16> = transpose(mat3x4<f16>(1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_8c06ce();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_8c06ce();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_8c06ce();
}
