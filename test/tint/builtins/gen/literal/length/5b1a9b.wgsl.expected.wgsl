enable f16;

fn length_5b1a9b() {
  var res : f16 = length(vec4<f16>(0.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_5b1a9b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_5b1a9b();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_5b1a9b();
}
