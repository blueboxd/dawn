enable f16;

fn round_f665b5() {
  var res : vec4<f16> = round(vec4<f16>(3.5h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_f665b5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_f665b5();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_f665b5();
}
