@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureSampleCompareLevel_1116ed() {
  var arg_2 = vec2<f32>();
  var arg_3 = 1i;
  var arg_4 = 1.0f;
  var res : f32 = textureSampleCompareLevel(arg_0, arg_1, arg_2, arg_3, arg_4);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleCompareLevel_1116ed();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleCompareLevel_1116ed();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleCompareLevel_1116ed();
}
