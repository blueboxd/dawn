// Copyright 2020 The Tint Authors.
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

#include "src/tint/symbol_table.h"

#include "src/tint/debug.h"

namespace tint {

SymbolTable::SymbolTable(tint::ProgramID program_id) : program_id_(program_id) {}

SymbolTable::SymbolTable(const SymbolTable&) = default;

SymbolTable::SymbolTable(SymbolTable&&) = default;

SymbolTable::~SymbolTable() = default;

SymbolTable& SymbolTable::operator=(const SymbolTable& other) = default;

SymbolTable& SymbolTable::operator=(SymbolTable&&) = default;

Symbol SymbolTable::Register(const std::string& name) {
    TINT_ASSERT(Symbol, !name.empty());

    auto it = name_to_symbol_.Find(name);
    if (it) {
        return *it;
    }

#if TINT_SYMBOL_STORE_DEBUG_NAME
    Symbol sym(next_symbol_, program_id_, name);
#else
    Symbol sym(next_symbol_, program_id_);
#endif
    ++next_symbol_;

    name_to_symbol_.Add(name, sym);
    symbol_to_name_.Add(sym, name);

    return sym;
}

Symbol SymbolTable::Get(const std::string& name) const {
    auto it = name_to_symbol_.Find(name);
    return it ? *it : Symbol();
}

std::string SymbolTable::NameFor(const Symbol symbol) const {
    TINT_ASSERT_PROGRAM_IDS_EQUAL(Symbol, program_id_, symbol);
    auto it = symbol_to_name_.Find(symbol);
    if (!it) {
        return symbol.to_str();
    }

    return *it;
}

Symbol SymbolTable::New(std::string prefix /* = "" */) {
    if (prefix.empty()) {
        prefix = "tint_symbol";
    }
    auto it = name_to_symbol_.Find(prefix);
    if (!it) {
        return Register(prefix);
    }

    size_t i = 0;
    auto last_prefix = last_prefix_to_index_.Find(prefix);
    if (last_prefix) {
        i = *last_prefix;
    }

    std::string name;
    do {
        ++i;
        name = prefix + "_" + std::to_string(i);
    } while (name_to_symbol_.Contains(name));

    if (last_prefix) {
        *last_prefix = i;
    } else {
        last_prefix_to_index_.Add(prefix, i);
    }

    return Register(name);
}

}  // namespace tint
