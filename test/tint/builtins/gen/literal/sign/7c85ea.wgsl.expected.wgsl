enable f16;

fn sign_7c85ea() {
  var res : f16 = sign(1.0h);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_7c85ea();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_7c85ea();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_7c85ea();
}
