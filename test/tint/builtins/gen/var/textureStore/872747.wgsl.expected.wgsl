@group(1) @binding(0) var arg_0 : texture_storage_1d<rg32float, write>;

fn textureStore_872747() {
  var arg_1 = 1i;
  var arg_2 = vec4<f32>(1.0f);
  textureStore(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_872747();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_872747();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_872747();
}
