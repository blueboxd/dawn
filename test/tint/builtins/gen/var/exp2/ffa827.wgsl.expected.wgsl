enable f16;

fn exp2_ffa827() {
  var arg_0 = vec4<f16>(1.0h);
  var res : vec4<f16> = exp2(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_ffa827();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp2_ffa827();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_ffa827();
}
