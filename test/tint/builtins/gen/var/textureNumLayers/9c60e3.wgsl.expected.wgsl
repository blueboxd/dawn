enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32uint, read>;

fn textureNumLayers_9c60e3() {
  var res : u32 = textureNumLayers(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLayers_9c60e3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLayers_9c60e3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLayers_9c60e3();
}
