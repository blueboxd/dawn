@group(1) @binding(0) var arg_0 : texture_2d_array<i32>;

fn textureNumLevels_adc783() {
  var res : u32 = textureNumLevels(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLevels_adc783();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLevels_adc783();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLevels_adc783();
}
