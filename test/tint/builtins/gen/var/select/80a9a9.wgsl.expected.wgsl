fn select_80a9a9() {
  var arg_0 = vec3<bool>();
  var arg_1 = vec3<bool>();
  var arg_2 = vec3<bool>();
  var res : vec3<bool> = select(arg_0, arg_1, arg_2);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_80a9a9();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  select_80a9a9();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  select_80a9a9();
}