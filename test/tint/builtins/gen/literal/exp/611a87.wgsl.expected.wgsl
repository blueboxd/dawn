enable f16;

fn exp_611a87() {
  var res : vec4<f16> = exp(vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp_611a87();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp_611a87();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_611a87();
}
