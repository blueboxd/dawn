@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

fn textureLoad_46a93f() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<u32>(), 1i, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_46a93f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_46a93f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_46a93f();
}
