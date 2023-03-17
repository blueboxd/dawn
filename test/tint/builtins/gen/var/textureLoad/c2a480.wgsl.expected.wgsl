@group(1) @binding(0) var arg_0 : texture_2d<i32>;

fn textureLoad_c2a480() {
  var arg_1 = vec2<i32>(1i);
  var arg_2 = 1i;
  var res : vec4<i32> = textureLoad(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_c2a480();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_c2a480();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_c2a480();
}
