enable f16;

fn select_830dd9() {
  var arg_0 = vec4<f16>(1.0h);
  var arg_1 = vec4<f16>(1.0h);
  var arg_2 = true;
  var res : vec4<f16> = select(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_830dd9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_830dd9();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_830dd9();
}
