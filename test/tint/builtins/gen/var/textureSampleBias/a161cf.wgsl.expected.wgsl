@group(1) @binding(0) var arg_0 : texture_2d<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleBias_a161cf() {
  var arg_2 = vec2<f32>(1.0f);
  var arg_3 = 1.0f;
  const arg_4 = vec2<i32>(1i);
  var res : vec4<f32> = textureSampleBias(arg_0, arg_1, arg_2, arg_3, arg_4);
}

@fragment
fn fragment_main() {
  textureSampleBias_a161cf();
}
