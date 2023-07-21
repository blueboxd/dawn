// Copyright 2023 The Tint Authors.
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

#include "src/tint/writer/ir_text_generator.h"

#include "src/tint/utils/containers/map.h"

namespace tint::writer {

IRTextGenerator::IRTextGenerator(ir::Module* mod) : ir_(mod) {}

IRTextGenerator::~IRTextGenerator() = default;

std::string IRTextGenerator::UniqueIdentifier(const std::string& prefix) {
    return ir_->symbols.New(prefix).Name();
}

}  // namespace tint::writer
