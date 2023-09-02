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

var<workgroup> arg_0: atomic<u32>;

// fn atomicMax(ptr<workgroup, atomic<u32>, read_write>, u32) -> u32
fn atomicMax_beccfc() {
  var arg_1 = 1u;
  var res: u32 = atomicMax(&arg_0, arg_1);
  prevent_dce = res;
}
@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@compute @workgroup_size(1)
fn compute_main() {
  atomicMax_beccfc();
}
