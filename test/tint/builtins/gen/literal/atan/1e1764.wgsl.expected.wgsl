enable f16;

fn atan_1e1764() {
  var res : vec2<f16> = atan(vec2<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_1e1764();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_1e1764();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_1e1764();
}
