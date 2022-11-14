enable f16;

fn mix_c1aec6() {
  var res : vec3<f16> = mix(vec3<f16>(1.0h), vec3<f16>(1.0h), 1.0h);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_c1aec6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_c1aec6();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_c1aec6();
}
