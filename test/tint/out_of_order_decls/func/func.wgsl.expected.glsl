#version 310 es
precision highp float;

void f2() {
}

void f1() {
  f2();
}

void main() {
  f1();
  return;
}
