@group(1) @binding(0) var arg_0 : texture_storage_2d<rg32float, read_write>;

fn textureStore_ab788e() {
  var arg_1 = vec2<i32>(1i);
  var arg_2 = vec4<f32>(1.0f);
  textureStore(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_ab788e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_ab788e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_ab788e();
}
