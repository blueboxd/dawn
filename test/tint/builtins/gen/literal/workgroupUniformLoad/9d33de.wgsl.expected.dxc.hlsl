groupshared int arg_0;

int tint_workgroupUniformLoad_arg_0() {
  GroupMemoryBarrierWithGroupSync();
  int result = arg_0;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void workgroupUniformLoad_9d33de() {
  int res = tint_workgroupUniformLoad_arg_0();
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  {
    arg_0 = 0;
  }
  GroupMemoryBarrierWithGroupSync();
  workgroupUniformLoad_9d33de();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
