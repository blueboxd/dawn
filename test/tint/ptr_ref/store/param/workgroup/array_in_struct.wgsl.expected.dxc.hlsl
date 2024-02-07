struct str {
  int arr[4];
};

groupshared str S;

void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      S.arr[i] = 0;
    }
  }
  GroupMemoryBarrierWithGroupSync();
}

void func_S_arr() {
  int tint_symbol_2[4] = (int[4])0;
  S.arr = tint_symbol_2;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  func_S_arr();
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
