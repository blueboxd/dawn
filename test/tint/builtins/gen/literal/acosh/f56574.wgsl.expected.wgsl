enable f16;

fn acosh_f56574() {
  var res : vec3<f16> = acosh(vec3<f16>(2.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_f56574();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_f56574();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_f56574();
}
