#version 310 es

void deref_const() {
  ivec3 a = ivec3(0, 0, 0);
  int b = a[0];
  a[0] = 42;
}

void no_deref_const() {
  ivec3 a = ivec3(0, 0, 0);
  int b = a[0];
  a[0] = 42;
}

void deref_let() {
  ivec3 a = ivec3(0, 0, 0);
  int i = 0;
  int b = a[i];
  a[0] = 42;
}

void no_deref_let() {
  ivec3 a = ivec3(0, 0, 0);
  int i = 0;
  int b = a[i];
  a[0] = 42;
}

void deref_var() {
  ivec3 a = ivec3(0, 0, 0);
  int i = 0;
  int b = a[i];
  a[0] = 42;
}

void no_deref_var() {
  ivec3 a = ivec3(0, 0, 0);
  int i = 0;
  int b = a[i];
  a[0] = 42;
}

void tint_symbol() {
  deref_const();
  no_deref_const();
  deref_let();
  no_deref_let();
  deref_var();
  no_deref_var();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
