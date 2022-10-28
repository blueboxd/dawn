@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

fn textureDimensions_3fc3dc() {
  var res : vec2<u32> = textureDimensions(arg_0, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_3fc3dc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_3fc3dc();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_3fc3dc();
}
