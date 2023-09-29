enable chromium_experimental_subgroups;

fn subgroupBroadcast_08beca() {
  var res : f32 = subgroupBroadcast(1.0f, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_08beca();
}
