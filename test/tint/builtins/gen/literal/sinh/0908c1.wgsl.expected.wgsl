enable f16;

fn sinh_0908c1() {
  var res : vec3<f16> = sinh(vec3<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_0908c1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_0908c1();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_0908c1();
}
