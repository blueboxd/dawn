@group(1) @binding(0) var arg_0 : texture_1d<f32>;

fn textureLoad_81c381() {
  var arg_1 = 1;
  var arg_2 = 0;
  var res : vec4<f32> = textureLoad(arg_0, arg_1, arg_2);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_81c381();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  textureLoad_81c381();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  textureLoad_81c381();
}