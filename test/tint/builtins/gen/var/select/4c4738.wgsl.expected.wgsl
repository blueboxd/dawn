fn select_4c4738() {
  const arg_0 = vec4(1);
  const arg_1 = vec4(1);
  var arg_2 = vec4<bool>(true);
  var res = select(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_4c4738();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_4c4738();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_4c4738();
}
