enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @builtin(position)
  pos : vec4f,
}

@fragment
fn f(in : In, @location(0) uv : vec4f) {
  P.a += (u32(in.pos.x) + u32(uv.x));
}
