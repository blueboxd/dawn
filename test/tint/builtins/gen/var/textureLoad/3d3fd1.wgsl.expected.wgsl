@group(1) @binding(0) var arg_0 : texture_2d_array<i32>;

fn textureLoad_3d3fd1() {
  var arg_1 = vec2<u32>();
  var arg_2 = 1u;
  var arg_3 = 1i;
  var res : vec4<i32> = textureLoad(arg_0, arg_1, arg_2, arg_3);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_3d3fd1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_3d3fd1();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_3d3fd1();
}
