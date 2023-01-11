// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

////////////////////////////////////////////////////////////////////////////////
// File generated by tools/src/cmd/gen
// using the template:
//   test/tint/builtins/gen/gen.wgsl.tmpl
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

@group(1) @binding(1) var arg_1: texture_2d_array<u32>;
@group(1) @binding(2) var arg_2: sampler;

// fn textureGather(@const component: i32, texture: texture_2d_array<u32>, sampler: sampler, coords: vec2<f32>, array_index: u32) -> vec4<u32>
fn textureGather_a0372b() {
  const arg_0 = 1i;
  var arg_3 = vec2<f32>(1.f);
  var arg_4 = 1u;
  var res: vec4<u32> = textureGather(arg_0, arg_1, arg_2, arg_3, arg_4);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_a0372b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_a0372b();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_a0372b();
}
