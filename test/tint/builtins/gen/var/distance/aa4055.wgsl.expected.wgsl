fn distance_aa4055() {
  var arg_0 = vec2<f32>();
  var arg_1 = vec2<f32>();
  var res : f32 = distance(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_aa4055();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  distance_aa4055();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  distance_aa4055();
}