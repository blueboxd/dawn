@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32float, read_write>;

fn textureStore_55f9dc() {
  textureStore(arg_0, vec2<u32>(1u), 1u, vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_55f9dc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_55f9dc();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_55f9dc();
}
