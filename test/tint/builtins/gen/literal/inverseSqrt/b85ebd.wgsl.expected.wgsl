enable f16;

fn inverseSqrt_b85ebd() {
  var res : vec3<f16> = inverseSqrt(vec3<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  inverseSqrt_b85ebd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  inverseSqrt_b85ebd();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_b85ebd();
}
