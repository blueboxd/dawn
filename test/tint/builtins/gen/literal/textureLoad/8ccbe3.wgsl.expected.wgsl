@group(1) @binding(0) var arg_0 : texture_depth_2d;

fn textureLoad_8ccbe3() {
  var res : f32 = textureLoad(arg_0, vec2<i32>(1i), 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_8ccbe3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_8ccbe3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_8ccbe3();
}
