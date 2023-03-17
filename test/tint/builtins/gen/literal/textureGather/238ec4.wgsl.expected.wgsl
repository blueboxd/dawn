@group(1) @binding(1) var arg_1 : texture_2d_array<f32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_238ec4() {
  var res : vec4<f32> = textureGather(1u, arg_1, arg_2, vec2<f32>(1.0f), 1u, vec2<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_238ec4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_238ec4();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_238ec4();
}
