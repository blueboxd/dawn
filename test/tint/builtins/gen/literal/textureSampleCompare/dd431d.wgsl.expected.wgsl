@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureSampleCompare_dd431d() {
  var res : f32 = textureSampleCompare(arg_0, arg_1, vec2<f32>(1.0f), 1i, 1.0f);
}

@fragment
fn fragment_main() {
  textureSampleCompare_dd431d();
}
