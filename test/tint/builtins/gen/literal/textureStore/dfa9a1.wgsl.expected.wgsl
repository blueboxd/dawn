@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rg32float, write>;

fn textureStore_dfa9a1() {
  textureStore(arg_0, vec2<u32>(1u), 1i, vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_dfa9a1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_dfa9a1();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_dfa9a1();
}
