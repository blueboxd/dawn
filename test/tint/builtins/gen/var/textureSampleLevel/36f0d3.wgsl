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
// File generated by 'tools/src/cmd/gen' using the template:
//   test/tint/builtins/gen/gen.wgsl.tmpl
//
// To regenerate run: './tools/run gen'
//
//                       Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

@group(1) @binding(0) var arg_0: texture_depth_2d_array;
@group(1) @binding(1) var arg_1: sampler;

// fn textureSampleLevel(texture: texture_depth_2d_array, sampler: sampler, coords: vec2<f32>, array_index: i32, level: u32, @const offset: vec2<i32>) -> f32
fn textureSampleLevel_36f0d3() {
  var arg_2 = vec2<f32>(1.f);
  var arg_3 = 1i;
  var arg_4 = 1u;
  const arg_5 = vec2<i32>(1i);
  var res: f32 = textureSampleLevel(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
  prevent_dce = res;
}
@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleLevel_36f0d3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleLevel_36f0d3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleLevel_36f0d3();
}
