SKIP: FAILED


enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

fn f0() {
  P.a += 9;
}

fn f1() {
  f0();
  P.a += 8;
}

fn f2() {
  P.a += 7;
  f1();
}

@fragment
fn f() -> @location(0) vec4f {
  f2();
  return vec4f(2);
}

Failed to generate: extensions/pixel_local/indirect_use/one_output/single_attachment.wgsl:2:8 error: HLSL backend does not support extension 'chromium_experimental_pixel_local'
enable chromium_experimental_pixel_local;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

