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

#include "src/tint/reader/wgsl/token.h"

namespace tint::reader::wgsl {

// static
std::string_view Token::TypeToName(Type type) {
    switch (type) {
        case Token::Type::kError:
            return "error";
        case Token::Type::kEOF:
            return "end of file";
        case Token::Type::kIdentifier:
            return "identifier";
        case Token::Type::kFloatLiteral:
            return "abstract float literal";
        case Token::Type::kFloatLiteral_F:
            return "'f'-suffixed float literal";
        case Token::Type::kFloatLiteral_H:
            return "'h'-suffixed float literal";
        case Token::Type::kIntLiteral:
            return "abstract integer literal";
        case Token::Type::kIntLiteral_I:
            return "'i'-suffixed integer literal";
        case Token::Type::kIntLiteral_U:
            return "'u'-suffixed integer literal";
        case Token::Type::kPlaceholder:
            return "placeholder";
        case Token::Type::kUninitialized:
            return "uninitialized";

        case Token::Type::kAnd:
            return "&";
        case Token::Type::kAndAnd:
            return "&&";
        case Token::Type::kArrow:
            return "->";
        case Token::Type::kAttr:
            return "@";
        case Token::Type::kForwardSlash:
            return "/";
        case Token::Type::kBang:
            return "!";
        case Token::Type::kBracketLeft:
            return "[";
        case Token::Type::kBracketRight:
            return "]";
        case Token::Type::kBraceLeft:
            return "{";
        case Token::Type::kBraceRight:
            return "}";
        case Token::Type::kColon:
            return ":";
        case Token::Type::kComma:
            return ",";
        case Token::Type::kEqual:
            return "=";
        case Token::Type::kEqualEqual:
            return "==";
        case Token::Type::kTemplateArgsRight:
        case Token::Type::kGreaterThan:
            return ">";
        case Token::Type::kGreaterThanEqual:
            return ">=";
        case Token::Type::kShiftRight:
            return ">>";
        case Token::Type::kTemplateArgsLeft:
        case Token::Type::kLessThan:
            return "<";
        case Token::Type::kLessThanEqual:
            return "<=";
        case Token::Type::kShiftLeft:
            return "<<";
        case Token::Type::kMod:
            return "%";
        case Token::Type::kNotEqual:
            return "!=";
        case Token::Type::kMinus:
            return "-";
        case Token::Type::kMinusMinus:
            return "--";
        case Token::Type::kPeriod:
            return ".";
        case Token::Type::kPlus:
            return "+";
        case Token::Type::kPlusPlus:
            return "++";
        case Token::Type::kOr:
            return "|";
        case Token::Type::kOrOr:
            return "||";
        case Token::Type::kParenLeft:
            return "(";
        case Token::Type::kParenRight:
            return ")";
        case Token::Type::kSemicolon:
            return ";";
        case Token::Type::kStar:
            return "*";
        case Token::Type::kTilde:
            return "~";
        case Token::Type::kUnderscore:
            return "_";
        case Token::Type::kXor:
            return "^";
        case Token::Type::kPlusEqual:
            return "+=";
        case Token::Type::kMinusEqual:
            return "-=";
        case Token::Type::kTimesEqual:
            return "*=";
        case Token::Type::kDivisionEqual:
            return "/=";
        case Token::Type::kModuloEqual:
            return "%=";
        case Token::Type::kAndEqual:
            return "&=";
        case Token::Type::kOrEqual:
            return "|=";
        case Token::Type::kXorEqual:
            return "^=";
        case Token::Type::kShiftLeftEqual:
            return "<<=";
        case Token::Type::kShiftRightEqual:
            return ">>=";

        case Token::Type::kAlias:
            return "alias";
        case Token::Type::kBitcast:
            return "bitcast";
        case Token::Type::kBreak:
            return "break";
        case Token::Type::kCase:
            return "case";
        case Token::Type::kConst:
            return "const";
        case Token::Type::kConstAssert:
            return "const_assert";
        case Token::Type::kContinue:
            return "continue";
        case Token::Type::kContinuing:
            return "continuing";
        case Token::Type::kDiagnostic:
            return "diagnostic";
        case Token::Type::kDiscard:
            return "discard";
        case Token::Type::kDefault:
            return "default";
        case Token::Type::kElse:
            return "else";
        case Token::Type::kEnable:
            return "enable";
        case Token::Type::kFallthrough:
            return "fallthrough";
        case Token::Type::kFalse:
            return "false";
        case Token::Type::kFn:
            return "fn";
        case Token::Type::kFor:
            return "for";
        case Token::Type::kIf:
            return "if";
        case Token::Type::kLet:
            return "let";
        case Token::Type::kLoop:
            return "loop";
        case Token::Type::kOverride:
            return "override";
        case Token::Type::kReturn:
            return "return";
        case Token::Type::kRequires:
            return "requires";
        case Token::Type::kStruct:
            return "struct";
        case Token::Type::kSwitch:
            return "switch";
        case Token::Type::kTrue:
            return "true";
        case Token::Type::kVar:
            return "var";
        case Token::Type::kWhile:
            return "while";
    }

    return "<unknown>";
}

Token::Token() : type_(Type::kUninitialized) {}

Token::Token(Type type, const Source& source, const std::string_view& view)
    : type_(type), source_(source), value_(view) {}

Token::Token(Type type, const Source& source, const std::string& str)
    : type_(type), source_(source), value_(str) {}

Token::Token(Type type, const Source& source, const char* str)
    : type_(type), source_(source), value_(std::string_view(str)) {}

Token::Token(Type type, const Source& source, int64_t val)
    : type_(type), source_(source), value_(val) {}

Token::Token(Type type, const Source& source, double val)
    : type_(type), source_(source), value_(val) {}

Token::Token(Type type, const Source& source) : type_(type), source_(source) {}

Token::Token(Token&&) = default;

Token::~Token() = default;

bool Token::operator==(std::string_view ident) const {
    if (type_ != Type::kIdentifier) {
        return false;
    }
    if (auto* view = std::get_if<std::string_view>(&value_)) {
        return *view == ident;
    }
    return std::get<std::string>(value_) == ident;
}

std::string Token::to_str() const {
    switch (type_) {
        case Type::kFloatLiteral:
            return std::to_string(std::get<double>(value_));
        case Type::kFloatLiteral_F:
            return std::to_string(std::get<double>(value_)) + "f";
        case Type::kFloatLiteral_H:
            return std::to_string(std::get<double>(value_)) + "h";
        case Type::kIntLiteral:
            return std::to_string(std::get<int64_t>(value_));
        case Type::kIntLiteral_I:
            return std::to_string(std::get<int64_t>(value_)) + "i";
        case Type::kIntLiteral_U:
            return std::to_string(std::get<int64_t>(value_)) + "u";
        case Type::kIdentifier:
        case Type::kError:
            if (auto* view = std::get_if<std::string_view>(&value_)) {
                return std::string(*view);
            }
            return std::get<std::string>(value_);
        default:
            return "";
    }
}

std::string_view Token::to_str_view() const {
    if (type_ != Type::kIdentifier) {
        return {};
    }

    if (auto* view = std::get_if<std::string_view>(&value_)) {
        return *view;
    }
    auto& s = std::get<std::string>(value_);
    return {s.data(), s.length()};
}

double Token::to_f64() const {
    return std::get<double>(value_);
}

int64_t Token::to_i64() const {
    return std::get<int64_t>(value_);
}

}  // namespace tint::reader::wgsl
