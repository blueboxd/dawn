struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};
RWByteAddressBuffer sb_rw : register(u0);

atomic_compare_exchange_result_i32 sb_rwatomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_result_i32 result=(atomic_compare_exchange_result_i32)0;
  sb_rw.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void atomicCompareExchangeWeak_1bd40a() {
  int arg_1 = 1;
  int arg_2 = 1;
  atomic_compare_exchange_result_i32 res = sb_rwatomicCompareExchangeWeak(0u, arg_1, arg_2);
}

void fragment_main() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}
