@group(1) @binding(0) var arg_0 : texture_storage_3d<r32sint, write>;

fn textureStore_f1e6d3() {
  textureStore(arg_0, vec3<u32>(), vec4<i32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_f1e6d3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_f1e6d3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_f1e6d3();
}
