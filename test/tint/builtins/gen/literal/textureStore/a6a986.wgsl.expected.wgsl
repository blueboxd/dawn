@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba16float, write>;

fn textureStore_a6a986() {
  textureStore(arg_0, vec2<u32>(), 1i, vec4<f32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_a6a986();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_a6a986();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_a6a986();
}
