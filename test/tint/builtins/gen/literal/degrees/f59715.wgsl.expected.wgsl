enable f16;

fn degrees_f59715() {
  var res : vec2<f16> = degrees(vec2<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_f59715();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  degrees_f59715();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_f59715();
}
