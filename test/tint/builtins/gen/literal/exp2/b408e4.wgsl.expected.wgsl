enable f16;

fn exp2_b408e4() {
  var res : f16 = exp2(1.0h);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_b408e4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp2_b408e4();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_b408e4();
}
