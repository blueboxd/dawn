enable f16;

fn mix_98ee3e() {
  var arg_0 = vec2<f16>(1.0h);
  var arg_1 = vec2<f16>(1.0h);
  var arg_2 = vec2<f16>(1.0h);
  var res : vec2<f16> = mix(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_98ee3e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_98ee3e();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_98ee3e();
}
