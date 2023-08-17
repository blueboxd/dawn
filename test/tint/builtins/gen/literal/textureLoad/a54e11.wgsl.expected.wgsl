enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba8sint, read_write>;

fn textureLoad_a54e11() {
  var res : vec4<i32> = textureLoad(arg_0, vec2<i32>(1i), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_a54e11();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_a54e11();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_a54e11();
}
