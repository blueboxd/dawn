enable f16;

fn log2_776088() {
  var res : vec4<f16> = log2(vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log2_776088();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log2_776088();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_776088();
}
