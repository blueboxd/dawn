@group(1) @binding(1) var arg_1 : texture_2d<f32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_5266da() {
  const arg_0 = 1i;
  var arg_3 = vec2<f32>(1.0f);
  var res : vec4<f32> = textureGather(arg_0, arg_1, arg_2, arg_3);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_5266da();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_5266da();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_5266da();
}
