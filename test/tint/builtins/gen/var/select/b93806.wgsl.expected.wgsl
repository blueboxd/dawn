fn select_b93806() {
  const arg_0 = vec3(1.0);
  const arg_1 = vec3(1.0);
  var arg_2 = vec3<bool>(true);
  var res = select(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_b93806();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_b93806();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_b93806();
}
