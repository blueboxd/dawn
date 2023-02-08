// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_TINT_SEM_TYPE_INITIALIZER_H_
#define SRC_TINT_SEM_TYPE_INITIALIZER_H_

#include "src/tint/sem/call_target.h"
#include "src/tint/utils/vector.h"

namespace tint::sem {

/// TypeInitializer is the CallTarget for a type initializer.
class TypeInitializer final : public Castable<TypeInitializer, CallTarget> {
  public:
    /// Constructor
    /// @param type the type that's being constructed
    /// @param parameters the type initializer parameters
    /// @param stage the earliest evaluation stage for the expression
    TypeInitializer(const type::Type* type,
                    utils::VectorRef<const Parameter*> parameters,
                    EvaluationStage stage);

    /// Destructor
    ~TypeInitializer() override;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_TYPE_INITIALIZER_H_
