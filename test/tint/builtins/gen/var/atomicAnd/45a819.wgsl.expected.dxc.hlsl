groupshared int arg_0;

void tint_zero_workgroup_memory(uint local_idx) {
  {
    int atomic_result = 0;
    InterlockedExchange(arg_0, 0, atomic_result);
  }
  GroupMemoryBarrierWithGroupSync();
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void atomicAnd_45a819() {
  int arg_1 = 1;
  int atomic_result_1 = 0;
  InterlockedAnd(arg_0, arg_1, atomic_result_1);
  int res = atomic_result_1;
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  atomicAnd_45a819();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
