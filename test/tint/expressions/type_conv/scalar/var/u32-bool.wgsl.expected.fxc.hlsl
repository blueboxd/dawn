[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint u = 1u;

void f() {
  const bool v = bool(u);
}
