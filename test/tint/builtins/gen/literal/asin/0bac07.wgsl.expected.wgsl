fn asin_0bac07() {
  var res = asin(vec3(0.479425538604));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_0bac07();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_0bac07();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_0bac07();
}
