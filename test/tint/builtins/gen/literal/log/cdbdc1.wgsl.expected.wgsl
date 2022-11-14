enable f16;

fn log_cdbdc1() {
  var res : vec4<f16> = log(vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log_cdbdc1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log_cdbdc1();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_cdbdc1();
}
