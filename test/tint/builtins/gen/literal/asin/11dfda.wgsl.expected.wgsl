enable f16;

fn asin_11dfda() {
  var res : f16 = asin(0.479248047h);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_11dfda();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_11dfda();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_11dfda();
}
