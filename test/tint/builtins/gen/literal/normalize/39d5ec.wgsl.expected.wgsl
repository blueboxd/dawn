enable f16;

fn normalize_39d5ec() {
  var res : vec3<f16> = normalize(vec3<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  normalize_39d5ec();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  normalize_39d5ec();
}

@compute @workgroup_size(1)
fn compute_main() {
  normalize_39d5ec();
}
