fn ceil_678655() {
  var arg_0 = 1.5f;
  var res : f32 = ceil(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_678655();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_678655();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_678655();
}
