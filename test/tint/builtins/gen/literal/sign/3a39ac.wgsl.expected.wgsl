fn sign_3a39ac() {
  var res = sign(1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_3a39ac();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_3a39ac();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_3a39ac();
}
