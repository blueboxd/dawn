enable f16;

fn cosh_b1b8a0() {
  var res : vec3<f16> = cosh(vec3<f16>(0.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cosh_b1b8a0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cosh_b1b8a0();
}

@compute @workgroup_size(1)
fn compute_main() {
  cosh_b1b8a0();
}
