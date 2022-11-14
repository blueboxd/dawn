enable f16;

fn tanh_e8efb3() {
  var res : vec4<f16> = tanh(vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_e8efb3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_e8efb3();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_e8efb3();
}
