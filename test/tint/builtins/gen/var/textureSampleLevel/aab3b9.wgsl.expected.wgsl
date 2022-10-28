@group(1) @binding(0) var arg_0 : texture_cube_array<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_aab3b9() {
  var arg_2 = vec3<f32>();
  var arg_3 = 1u;
  var arg_4 = 1.0f;
  var res : vec4<f32> = textureSampleLevel(arg_0, arg_1, arg_2, arg_3, arg_4);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleLevel_aab3b9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleLevel_aab3b9();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleLevel_aab3b9();
}
