@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

fn textureLoad_66be47() {
  var res : f32 = textureLoad(arg_0, vec2<i32>(), 1u, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_66be47();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_66be47();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_66be47();
}
