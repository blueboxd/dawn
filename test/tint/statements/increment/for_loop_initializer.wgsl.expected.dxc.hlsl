[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer i : register(u0);

void main() {
  {
    for(i.Store(0u, asuint((i.Load(0u) + 1u))); (i.Load(0u) < 10u); ) {
    }
  }
}
