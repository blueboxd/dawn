@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleBias_80e579() {
  var arg_2 = vec2<f32>(1.0f);
  var arg_3 = 1i;
  var arg_4 = 1.0f;
  var res : vec4<f32> = textureSampleBias(arg_0, arg_1, arg_2, arg_3, arg_4);
}

@fragment
fn fragment_main() {
  textureSampleBias_80e579();
}
