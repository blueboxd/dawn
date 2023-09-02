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


enable f16;

// fn degrees(vec<4, f16>) -> vec<4, f16>
fn degrees_3055d3() {
  var res: vec4<f16> = degrees(vec4<f16>(1.h));
  prevent_dce = res;
}
@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_3055d3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  degrees_3055d3();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_3055d3();
}
