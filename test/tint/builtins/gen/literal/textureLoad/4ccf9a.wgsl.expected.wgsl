@group(1) @binding(0) var arg_0 : texture_storage_3d<rg32uint, read_write>;

fn textureLoad_4ccf9a() {
  var res : vec4<u32> = textureLoad(arg_0, vec3<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_4ccf9a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_4ccf9a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_4ccf9a();
}
