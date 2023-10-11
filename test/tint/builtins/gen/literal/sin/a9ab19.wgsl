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


// fn sin(fa) -> fa
fn sin_a9ab19() {
  var res = sin(1.57079632679);
}
@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_a9ab19();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_a9ab19();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_a9ab19();
}
