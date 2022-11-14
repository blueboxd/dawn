enable f16;

fn faceForward_cc63dc() {
  var res : vec4<f16> = faceForward(vec4<f16>(1.0h), vec4<f16>(1.0h), vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  faceForward_cc63dc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  faceForward_cc63dc();
}

@compute @workgroup_size(1)
fn compute_main() {
  faceForward_cc63dc();
}
