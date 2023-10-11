// Copyright 2021 The Tint Authors.
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


// fn clamp(vec<4, ia>, vec<4, ia>, vec<4, ia>) -> vec<4, ia>
fn clamp_d396af() {
  const arg_0 = vec4(1);
  const arg_1 = vec4(1);
  const arg_2 = vec4(1);
  var res = clamp(arg_0, arg_1, arg_2);
}
@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_d396af();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_d396af();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_d396af();
}
