alias Arr = array<u32, 4u>;

var<private> local_invocation_index_1 : u32;

var<workgroup> wg : array<atomic<u32>, 4u>;

fn compute_main_inner(local_invocation_index : u32) {
  var idx : u32 = 0u;
  idx = local_invocation_index;
  loop {
    let x_21 : u32 = idx;
    if (!((x_21 < 4u))) {
      break;
    }
    let x_26 : u32 = idx;
    atomicStore(&(wg[x_26]), 0u);

    continuing {
      let x_33 : u32 = idx;
      idx = (x_33 + 1u);
    }
  }
  workgroupBarrier();
  atomicStore(&(wg[1i]), 1u);
  return;
}

fn compute_main_1() {
  let x_47 : u32 = local_invocation_index_1;
  compute_main_inner(x_47);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
