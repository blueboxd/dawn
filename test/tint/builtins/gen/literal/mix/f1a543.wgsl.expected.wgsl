enable f16;

fn mix_f1a543() {
  var res : vec4<f16> = mix(vec4<f16>(1.0h), vec4<f16>(1.0h), 1.0h);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_f1a543();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_f1a543();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_f1a543();
}
