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

@group(1) @binding(0) var arg_0: texture_2d_array<f32>;

// fn textureLoad(texture: texture_2d_array<f32>, coords: vec2<u32>, array_index: i32, level: i32) -> vec4<f32>
fn textureLoad_54a59b() {
  var arg_1 = vec2<u32>(1u);
  var arg_2 = 1i;
  var arg_3 = 1i;
  var res: vec4<f32> = textureLoad(arg_0, arg_1, arg_2, arg_3);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_54a59b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_54a59b();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_54a59b();
}
