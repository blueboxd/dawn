enable f16;

fn smoothstep_c43ebd() {
  var res : vec4<f16> = smoothstep(vec4<f16>(1.0h), vec4<f16>(1.0h), vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_c43ebd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_c43ebd();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_c43ebd();
}
