@group(1) @binding(0) var arg_0 : texture_storage_1d<r32uint, write>;

fn textureStore_a6e78f() {
  textureStore(arg_0, 1u, vec4<u32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_a6e78f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_a6e78f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_a6e78f();
}
