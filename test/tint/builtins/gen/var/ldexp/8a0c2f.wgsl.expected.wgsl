enable f16;

fn ldexp_8a0c2f() {
  var arg_0 = vec4<f16>(1.0h);
  const arg_1 = vec4(1);
  var res : vec4<f16> = ldexp(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_8a0c2f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_8a0c2f();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_8a0c2f();
}
