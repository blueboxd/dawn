enable f16;

fn ldexp_7485ce() {
  var arg_0 = vec3<f16>(1.0h);
  var arg_1 = vec3<i32>(1i);
  var res : vec3<f16> = ldexp(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_7485ce();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_7485ce();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_7485ce();
}
