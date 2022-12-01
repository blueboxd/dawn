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

#ifndef SRC_TINT_IR_INSTRUCTION_H_
#define SRC_TINT_IR_INSTRUCTION_H_

#include <ostream>

#include "src/tint/castable.h"

namespace tint::ir {

/// An instruction in the IR.
class Instruction : public Castable<Instruction> {
  public:
    Instruction(const Instruction& instr) = delete;
    Instruction(Instruction&& instr) = delete;
    /// Destructor
    ~Instruction() override;

    Instruction& operator=(const Instruction& instr) = delete;
    Instruction& operator=(Instruction&& instr) = delete;

    /// Write the instruction to the given stream
    /// @param out the stream to write to
    /// @returns the stream
    virtual std::ostream& ToString(std::ostream& out) const = 0;

  protected:
    /// Constructor
    Instruction();
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_INSTRUCTION_H_
