// flags: --pixel_local_attachments 0=1,1=6,2=3
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @invariant @builtin(position) pos : vec4f,
}

@fragment fn f(in : In) {
  P.a += u32(in.pos.x);
}
