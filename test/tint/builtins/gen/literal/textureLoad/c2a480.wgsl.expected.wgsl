@group(1) @binding(0) var arg_0 : texture_2d<i32>;

fn textureLoad_c2a480() {
  var res : vec4<i32> = textureLoad(arg_0, vec2<i32>(), 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_c2a480();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_c2a480();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_c2a480();
}
