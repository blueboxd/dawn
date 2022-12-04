enable chromium_experimental_full_ptr_parameters;

struct str {
  i : vec4<f32>,
};

@group(0) @binding(0) var<uniform> S : str;

fn func(pointer : ptr<uniform, vec4<f32>>) -> vec4<f32> {
  return *pointer;
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&S.i);
}
