enable f16;

fn asin_3cfbd4() {
  var res : vec4<f16> = asin(vec4<f16>(0.479248047h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_3cfbd4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_3cfbd4();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_3cfbd4();
}
