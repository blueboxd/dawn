enable f16;

fn degrees_3055d3() {
  var res : vec4<f16> = degrees(vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_3055d3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  degrees_3055d3();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_3055d3();
}
