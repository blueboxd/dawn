@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba8uint, read_write>;

fn textureStore_9d7c62() {
  var arg_1 = 1i;
  var arg_2 = vec4<u32>(1u);
  textureStore(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_9d7c62();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_9d7c62();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_9d7c62();
}
