@group(1) @binding(0) var arg_0 : texture_2d_array<i32>;

fn textureLoad_2363be() {
  var res : vec4<i32> = textureLoad(arg_0, vec2<u32>(), 1i, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_2363be();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_2363be();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_2363be();
}
