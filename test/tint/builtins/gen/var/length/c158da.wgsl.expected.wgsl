enable f16;

fn length_c158da() {
  var arg_0 = 1.0h;
  var res : f16 = length(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_c158da();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_c158da();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_c158da();
}
