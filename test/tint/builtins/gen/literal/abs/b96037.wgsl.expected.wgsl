fn abs_b96037() {
  var res : f32 = abs(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_b96037();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_b96037();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_b96037();
}
