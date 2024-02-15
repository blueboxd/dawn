@group(1) @binding(0) var arg_0 : texture_storage_1d<rg32sint, read>;

fn textureLoad_2d6cf7() {
  var arg_1 = 1i;
  var res : vec4<i32> = textureLoad(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_2d6cf7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_2d6cf7();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_2d6cf7();
}