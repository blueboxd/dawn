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


// fn transpose(mat<4, 3, f32>) -> mat<3, 4, f32>
fn transpose_2585cd() {
  var arg_0 = mat4x3<f32>(1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f);
  var res: mat3x4<f32> = transpose(arg_0);
  prevent_dce = res;
}
@group(2) @binding(0) var<storage, read_write> prevent_dce : mat3x4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_2585cd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_2585cd();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_2585cd();
}
