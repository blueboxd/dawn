fn fwidthCoarse_1e59d9() {
  var res : vec3<f32> = fwidthCoarse(vec3<f32>(1.0f));
}

@fragment
fn fragment_main() {
  fwidthCoarse_1e59d9();
}
