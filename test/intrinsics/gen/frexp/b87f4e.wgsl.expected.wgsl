var<workgroup> arg_1 : vec4<i32>;

fn frexp_b87f4e() {
  var res : vec4<f32> = frexp(vec4<f32>(), &(arg_1));
}

[[stage(compute)]]
fn compute_main() {
  frexp_b87f4e();
}
