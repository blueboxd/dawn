enable f16;

fn fract_498c77() {
  var arg_0 = vec4<f16>(1.0h);
  var res : vec4<f16> = fract(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_498c77();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_498c77();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_498c77();
}
