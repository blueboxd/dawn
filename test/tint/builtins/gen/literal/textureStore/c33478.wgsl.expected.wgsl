@group(1) @binding(0) var arg_0 : texture_storage_3d<rg32float, read_write>;

fn textureStore_c33478() {
  textureStore(arg_0, vec3<i32>(1i), vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_c33478();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_c33478();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_c33478();
}
