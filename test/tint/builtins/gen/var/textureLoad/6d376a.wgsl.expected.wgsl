@group(1) @binding(0) var arg_0 : texture_1d<f32>;

fn textureLoad_6d376a() {
  var arg_1 = 1u;
  var arg_2 = 1u;
  var res : vec4<f32> = textureLoad(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_6d376a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_6d376a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_6d376a();
}
