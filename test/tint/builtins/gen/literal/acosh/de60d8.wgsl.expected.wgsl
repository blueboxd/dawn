enable f16;

fn acosh_de60d8() {
  var res : vec4<f16> = acosh(vec4<f16>(2.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_de60d8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_de60d8();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_de60d8();
}
