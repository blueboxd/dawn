enable f16;

fn radians_44f20b() {
  var res : vec4<f16> = radians(vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_44f20b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  radians_44f20b();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_44f20b();
}
