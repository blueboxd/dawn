@group(1) @binding(0) var arg_0 : texture_3d<u32>;

fn textureLoad_92eb1f() {
  var arg_1 = vec3<u32>(1u);
  var arg_2 = 1i;
  var res : vec4<u32> = textureLoad(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_92eb1f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_92eb1f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_92eb1f();
}
