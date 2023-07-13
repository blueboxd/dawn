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

#include "src/tint/writer/syntax_tree/generator_impl.h"

#include <algorithm>

#include "src/tint/ast/alias.h"
#include "src/tint/ast/bool_literal_expression.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/float_literal_expression.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/internal_attribute.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/invariant_attribute.h"
#include "src/tint/ast/module.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/stride_attribute.h"
#include "src/tint/ast/struct_member_align_attribute.h"
#include "src/tint/ast/struct_member_offset_attribute.h"
#include "src/tint/ast/struct_member_size_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/switch_statement.h"
#include "src/tint/switch.h"
#include "src/tint/utils/math.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/writer/float_to_string.h"

namespace tint::writer::syntax_tree {

GeneratorImpl::GeneratorImpl(const Program* program) : ASTTextGenerator(program) {}

GeneratorImpl::~GeneratorImpl() = default;

void GeneratorImpl::Generate() {
    // Generate global declarations in the order they appear in the module.
    for (auto* decl : program_->AST().GlobalDeclarations()) {
        Switch(
            decl,  //
            [&](const ast::DiagnosticDirective* dd) { EmitDiagnosticControl(dd->control); },
            [&](const ast::Enable* e) { EmitEnable(e); },
            [&](const ast::TypeDecl* td) { EmitTypeDecl(td); },
            [&](const ast::Function* func) { EmitFunction(func); },
            [&](const ast::Variable* var) { EmitVariable(var); },
            [&](const ast::ConstAssert* ca) { EmitConstAssert(ca); },
            [&](Default) { TINT_UNREACHABLE(Writer, diagnostics_); });

        if (decl != program_->AST().GlobalDeclarations().Back()) {
            Line();
        }
    }
}

void GeneratorImpl::EmitDiagnosticControl(const ast::DiagnosticControl& diagnostic) {
    Line() << "DiagnosticControl [severity: " << diagnostic.severity
           << ", rule: " << diagnostic.rule_name->String() << "]";
}

void GeneratorImpl::EmitEnable(const ast::Enable* enable) {
    auto l = Line();
    l << "Enable [";
    for (auto* ext : enable->extensions) {
        if (ext != enable->extensions.Front()) {
            l << ", ";
        }
        l << ext->name;
    }
    l << "]";
}

void GeneratorImpl::EmitTypeDecl(const ast::TypeDecl* ty) {
    Switch(
        ty,  //
        [&](const ast::Alias* alias) {
            Line() << "Alias [";
            {
                ScopedIndent ai(this);

                Line() << "name: " << alias->name->symbol.Name();
                Line() << "expr: ";
                {
                    ScopedIndent ex(this);
                    EmitExpression(alias->type);
                }
            }
            Line() << "]";
        },
        [&](const ast::Struct* str) { EmitStructType(str); },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown declared type: " + std::string(ty->TypeInfo().name));
        });
}

void GeneratorImpl::EmitExpression(const ast::Expression* expr) {
    Switch(
        expr,  //
        [&](const ast::IndexAccessorExpression* a) { EmitIndexAccessor(a); },
        [&](const ast::BinaryExpression* b) { EmitBinary(b); },
        [&](const ast::BitcastExpression* b) { EmitBitcast(b); },
        [&](const ast::CallExpression* c) { EmitCall(c); },
        [&](const ast::IdentifierExpression* i) { EmitIdentifier(i); },
        [&](const ast::LiteralExpression* l) { EmitLiteral(l); },
        [&](const ast::MemberAccessorExpression* m) { EmitMemberAccessor(m); },
        [&](const ast::PhonyExpression*) { Line() << "[PhonyExpression]"; },
        [&](const ast::UnaryOpExpression* u) { EmitUnaryOp(u); },
        [&](Default) { diagnostics_.add_error(diag::System::Writer, "unknown expression type"); });
}

void GeneratorImpl::EmitIndexAccessor(const ast::IndexAccessorExpression* expr) {
    Line() << "IndexAccessorExpression [";
    {
        ScopedIndent iae(this);
        Line() << "object: ";
        {
            ScopedIndent obj(this);
            EmitExpression(expr->object);
        }

        Line() << "index: ";
        {
            ScopedIndent idx(this);
            EmitExpression(expr->index);
        }
    }
    Line() << "]";
}

void GeneratorImpl::EmitMemberAccessor(const ast::MemberAccessorExpression* expr) {
    Line() << "MemberAccessorExpression [";
    {
        ScopedIndent mae(this);

        Line() << "object: ";
        {
            ScopedIndent obj(this);
            EmitExpression(expr->object);
        }
        Line() << "member: " << expr->member->symbol.Name();
    }
    Line() << "]";
}

void GeneratorImpl::EmitBitcast(const ast::BitcastExpression* expr) {
    Line() << "BitcastExpression [";
    {
        ScopedIndent bc(this);
        {
            Line() << "type: ";
            ScopedIndent ty(this);
            EmitExpression(expr->type);
        }
        {
            Line() << "expr: ";
            ScopedIndent exp(this);
            EmitExpression(expr->expr);
        }
    }
    Line() << "]";
}

void GeneratorImpl::EmitCall(const ast::CallExpression* expr) {
    Line() << "Call [";
    {
        ScopedIndent cl(this);

        Line() << "target: [";
        {
            ScopedIndent tgt(this);
            EmitExpression(expr->target);
        }
        Line() << "]";

        if (!expr->args.IsEmpty()) {
            Line() << "args: [";
            {
                ScopedIndent args(this);
                for (auto* arg : expr->args) {
                    Line() << "arg: [";
                    {
                        ScopedIndent arg_val(this);
                        EmitExpression(arg);
                    }
                    Line() << "]";
                }
            }
            Line() << "]";
        }
    }
    Line() << "]";
}

void GeneratorImpl::EmitLiteral(const ast::LiteralExpression* lit) {
    Line() << "LiteralExpression [";
    {
        ScopedIndent le(this);
        Switch(
            lit,  //
            [&](const ast::BoolLiteralExpression* l) { Line() << (l->value ? "true" : "false"); },
            [&](const ast::FloatLiteralExpression* l) {
                // f16 literals are also emitted as float value with suffix "h".
                // Note that all normal and subnormal f16 values are normal f32 values, and since
                // NaN and Inf are not allowed to be spelled in literal, it should be fine to emit
                // f16 literals in this way.
                if (l->suffix == ast::FloatLiteralExpression::Suffix::kNone) {
                    Line() << DoubleToBitPreservingString(l->value);
                } else {
                    Line() << FloatToBitPreservingString(static_cast<float>(l->value)) << l->suffix;
                }
            },
            [&](const ast::IntLiteralExpression* l) { Line() << l->value << l->suffix; },
            [&](Default) { diagnostics_.add_error(diag::System::Writer, "unknown literal type"); });
    }
    Line() << "]";
}

void GeneratorImpl::EmitIdentifier(const ast::IdentifierExpression* expr) {
    Line() << "IdentifierExpression [";
    {
        ScopedIndent ie(this);
        EmitIdentifier(expr->identifier);
    }
    Line() << "]";
}

void GeneratorImpl::EmitIdentifier(const ast::Identifier* ident) {
    Line() << "Identifier [";
    {
        ScopedIndent id(this);
        if (auto* tmpl_ident = ident->As<ast::TemplatedIdentifier>()) {
            Line() << "Templated [";
            {
                ScopedIndent tmpl(this);
                if (!tmpl_ident->attributes.IsEmpty()) {
                    Line() << "attrs: [";
                    {
                        ScopedIndent attrs(this);
                        EmitAttributes(tmpl_ident->attributes);
                    }
                    Line() << "]";
                }
                Line() << "name: " << ident->symbol.Name();
                if (!tmpl_ident->arguments.IsEmpty()) {
                    Line() << "args: [";
                    {
                        ScopedIndent args(this);
                        for (auto* expr : tmpl_ident->arguments) {
                            EmitExpression(expr);
                        }
                    }
                    Line() << "]";
                }
            }
            Line() << "]";
        } else {
            Line() << ident->symbol.Name();
        }
    }
    Line() << "]";
}

void GeneratorImpl::EmitFunction(const ast::Function* func) {
    Line() << "Function [";
    {
        ScopedIndent funct(this);

        if (func->attributes.Length()) {
            Line() << "attrs: [";
            {
                ScopedIndent attrs(this);
                EmitAttributes(func->attributes);
            }
            Line() << "]";
        }
        Line() << "name: " << func->name->symbol.Name();

        if (!func->params.IsEmpty()) {
            Line() << "params: [";
            {
                ScopedIndent args(this);
                for (auto* v : func->params) {
                    Line() << "param: [";
                    {
                        ScopedIndent param(this);
                        Line() << "name: " << v->name->symbol.Name();
                        if (!v->attributes.IsEmpty()) {
                            Line() << "attrs: [";
                            {
                                ScopedIndent attrs(this);
                                EmitAttributes(v->attributes);
                            }
                            Line() << "]";
                        }
                        Line() << "type: [";
                        {
                            ScopedIndent ty(this);
                            EmitExpression(v->type);
                        }
                        Line() << "]";
                    }
                    Line() << "]";
                }
            }
            Line() << "]";
        }

        Line() << "return: [";
        {
            ScopedIndent ret(this);

            if (func->return_type || !func->return_type_attributes.IsEmpty()) {
                if (!func->return_type_attributes.IsEmpty()) {
                    Line() << "attrs: [";
                    {
                        ScopedIndent attrs(this);
                        EmitAttributes(func->return_type_attributes);
                    }
                    Line() << "]";
                }

                Line() << "type: [";
                {
                    ScopedIndent ty(this);
                    EmitExpression(func->return_type);
                }
                Line() << "]";
            } else {
                Line() << "void";
            }
        }
        Line() << "]";
        Line() << "body: [";
        {
            ScopedIndent bdy(this);
            if (func->body) {
                EmitBlockHeader(func->body);
                EmitStatementsWithIndent(func->body->statements);
            }
        }
        Line() << "]";
    }
    Line() << "]";
}

void GeneratorImpl::EmitImageFormat(const builtin::TexelFormat fmt) {
    Line() << "builtin::TexelFormat [" << fmt << "]";
}

void GeneratorImpl::EmitStructType(const ast::Struct* str) {
    Line() << "Struct [";
    {
        ScopedIndent strct(this);

        if (str->attributes.Length()) {
            Line() << "attrs: [";
            {
                ScopedIndent attrs(this);
                EmitAttributes(str->attributes);
            }
            Line() << "]";
        }
        Line() << "name: " << str->name->symbol.Name();
        Line() << "members: [";
        {
            ScopedIndent membs(this);

            for (auto* mem : str->members) {
                Line() << "StructMember[";
                {
                    ScopedIndent m(this);
                    if (!mem->attributes.IsEmpty()) {
                        Line() << "attrs: [";
                        {
                            ScopedIndent attrs(this);
                            EmitAttributes(mem->attributes);
                        }
                        Line() << "]";
                    }

                    Line() << "name: " << mem->name->symbol.Name();
                    Line() << "type: [";
                    {
                        ScopedIndent ty(this);
                        EmitExpression(mem->type);
                    }
                    Line() << "]";
                }
            }
            Line() << "]";
        }
        Line() << "]";
    }
    Line() << "]";
}

void GeneratorImpl::EmitVariable(const ast::Variable* v) {
    Line() << "Variable [";
    {
        ScopedIndent variable(this);
        if (!v->attributes.IsEmpty()) {
            Line() << "attrs: [";
            {
                ScopedIndent attr(this);
                EmitAttributes(v->attributes);
            }
            Line() << "]";
        }

        Switch(
            v,  //
            [&](const ast::Var* var) {
                if (var->declared_address_space || var->declared_access) {
                    Line() << "Var [";
                    {
                        ScopedIndent vr(this);
                        Line() << "address_space: [";
                        {
                            ScopedIndent addr(this);
                            EmitExpression(var->declared_address_space);
                        }
                        Line() << "]";
                        if (var->declared_access) {
                            Line() << "access: [";
                            {
                                ScopedIndent acs(this);
                                EmitExpression(var->declared_access);
                            }
                            Line() << "]";
                        }
                    }
                    Line() << "]";
                } else {
                    Line() << "Var []";
                }
            },
            [&](const ast::Let*) { Line() << "Let []"; },
            [&](const ast::Override*) { Line() << "Override []"; },
            [&](const ast::Const*) { Line() << "Const []"; },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_) << "unhandled variable type " << v->TypeInfo().name;
            });

        Line() << "name: " << v->name->symbol.Name();

        if (auto ty = v->type) {
            Line() << "type: [";
            {
                ScopedIndent vty(this);
                EmitExpression(ty);
            }
            Line() << "]";
        }

        if (v->initializer != nullptr) {
            Line() << "initializer: [";
            {
                ScopedIndent init(this);
                EmitExpression(v->initializer);
            }
            Line() << "]";
        }
    }
    Line() << "]";
}

void GeneratorImpl::EmitAttributes(utils::VectorRef<const ast::Attribute*> attrs) {
    for (auto* attr : attrs) {
        Switch(
            attr,  //
            [&](const ast::WorkgroupAttribute* workgroup) {
                auto values = workgroup->Values();
                Line() << "WorkgroupAttribute [";
                {
                    ScopedIndent wg(this);
                    for (size_t i = 0; i < 3; i++) {
                        if (values[i]) {
                            EmitExpression(values[i]);
                        }
                    }
                }
                Line() << "]";
            },
            [&](const ast::StageAttribute* stage) {
                Line() << "StageAttribute [" << stage->stage << "]";
            },
            [&](const ast::BindingAttribute* binding) {
                Line() << "BindingAttribute [";
                {
                    ScopedIndent ba(this);
                    EmitExpression(binding->expr);
                }
                Line() << "]";
            },
            [&](const ast::GroupAttribute* group) {
                Line() << "GroupAttribute [";
                {
                    ScopedIndent ga(this);
                    EmitExpression(group->expr);
                }
                Line() << "]";
            },
            [&](const ast::LocationAttribute* location) {
                Line() << "LocationAttribute [";
                {
                    ScopedIndent la(this);
                    EmitExpression(location->expr);
                }
                Line() << "]";
            },
            [&](const ast::IndexAttribute* index) {
                Line() << "IndexAttribute [";
                {
                    ScopedIndent idx(this);
                    EmitExpression(index->expr);
                }
                Line() << "]";
            },
            [&](const ast::BuiltinAttribute* builtin) {
                Line() << "BuiltinAttribute [";
                {
                    ScopedIndent ba(this);
                    EmitExpression(builtin->builtin);
                }
                Line() << "]";
            },
            [&](const ast::DiagnosticAttribute* diagnostic) {
                EmitDiagnosticControl(diagnostic->control);
            },
            [&](const ast::InterpolateAttribute* interpolate) {
                Line() << "InterpolateAttribute [";
                {
                    ScopedIndent ia(this);
                    Line() << "type: [";
                    {
                        ScopedIndent ty(this);
                        EmitExpression(interpolate->type);
                    }
                    Line() << "]";
                    if (interpolate->sampling) {
                        Line() << "sampling: [";
                        {
                            ScopedIndent sa(this);
                            EmitExpression(interpolate->sampling);
                        }
                        Line() << "]";
                    }
                }
                Line() << "]";
            },
            [&](const ast::InvariantAttribute*) { Line() << "InvariantAttribute []"; },
            [&](const ast::IdAttribute* override_deco) {
                Line() << "IdAttribute [";
                {
                    ScopedIndent id(this);
                    EmitExpression(override_deco->expr);
                }
                Line() << "]";
            },
            [&](const ast::MustUseAttribute*) { Line() << "MustUseAttribute []"; },
            [&](const ast::StructMemberOffsetAttribute* offset) {
                Line() << "StructMemberOffsetAttribute [";
                {
                    ScopedIndent smoa(this);
                    EmitExpression(offset->expr);
                }
                Line() << "]";
            },
            [&](const ast::StructMemberSizeAttribute* size) {
                Line() << "StructMemberSizeAttribute [";
                {
                    ScopedIndent smsa(this);
                    EmitExpression(size->expr);
                }
                Line() << "]";
            },
            [&](const ast::StructMemberAlignAttribute* align) {
                Line() << "StructMemberAlignAttribute [";
                {
                    ScopedIndent smaa(this);
                    EmitExpression(align->expr);
                }
                Line() << "]";
            },
            [&](const ast::StrideAttribute* stride) {
                Line() << "StrideAttribute [" << stride->stride << "]";
            },
            [&](const ast::InternalAttribute* internal) {
                Line() << "InternalAttribute [" << internal->InternalName() << "]";
            },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_)
                    << "Unsupported attribute '" << attr->TypeInfo().name << "'";
            });
    }
}

void GeneratorImpl::EmitBinary(const ast::BinaryExpression* expr) {
    Line() << "BinaryExpression [";
    {
        ScopedIndent be(this);
        Line() << "lhs: [";
        {
            ScopedIndent lhs(this);

            EmitExpression(expr->lhs);
        }
        Line() << "]";
        Line() << "op: [";
        {
            ScopedIndent op(this);
            EmitBinaryOp(expr->op);
        }
        Line() << "]";
        Line() << "rhs: [";
        {
            ScopedIndent rhs(this);
            EmitExpression(expr->rhs);
        }
        Line() << "]";
    }
    Line() << "]";
}

void GeneratorImpl::EmitBinaryOp(const ast::BinaryOp op) {
    switch (op) {
        case ast::BinaryOp::kAnd:
            Line() << "&";
            break;
        case ast::BinaryOp::kOr:
            Line() << "|";
            break;
        case ast::BinaryOp::kXor:
            Line() << "^";
            break;
        case ast::BinaryOp::kLogicalAnd:
            Line() << "&&";
            break;
        case ast::BinaryOp::kLogicalOr:
            Line() << "||";
            break;
        case ast::BinaryOp::kEqual:
            Line() << "==";
            break;
        case ast::BinaryOp::kNotEqual:
            Line() << "!=";
            break;
        case ast::BinaryOp::kLessThan:
            Line() << "<";
            break;
        case ast::BinaryOp::kGreaterThan:
            Line() << ">";
            break;
        case ast::BinaryOp::kLessThanEqual:
            Line() << "<=";
            break;
        case ast::BinaryOp::kGreaterThanEqual:
            Line() << ">=";
            break;
        case ast::BinaryOp::kShiftLeft:
            Line() << "<<";
            break;
        case ast::BinaryOp::kShiftRight:
            Line() << ">>";
            break;
        case ast::BinaryOp::kAdd:
            Line() << "+";
            break;
        case ast::BinaryOp::kSubtract:
            Line() << "-";
            break;
        case ast::BinaryOp::kMultiply:
            Line() << "*";
            break;
        case ast::BinaryOp::kDivide:
            Line() << "/";
            break;
        case ast::BinaryOp::kModulo:
            Line() << "%";
            break;
        case ast::BinaryOp::kNone:
            diagnostics_.add_error(diag::System::Writer, "missing binary operation type");
            break;
    }
}

void GeneratorImpl::EmitUnaryOp(const ast::UnaryOpExpression* expr) {
    Line() << "UnaryOpExpression [";
    {
        ScopedIndent uoe(this);
        Line() << "op: [";
        {
            ScopedIndent op(this);
            switch (expr->op) {
                case ast::UnaryOp::kAddressOf:
                    Line() << "&";
                    break;
                case ast::UnaryOp::kComplement:
                    Line() << "~";
                    break;
                case ast::UnaryOp::kIndirection:
                    Line() << "*";
                    break;
                case ast::UnaryOp::kNot:
                    Line() << "!";
                    break;
                case ast::UnaryOp::kNegation:
                    Line() << "-";
                    break;
            }
        }
        Line() << "]";
        Line() << "expr: [";
        {
            ScopedIndent ex(this);
            EmitExpression(expr->expr);
        }
        Line() << "]";
    }
    Line() << "]";
}

void GeneratorImpl::EmitBlock(const ast::BlockStatement* stmt) {
    EmitBlockHeader(stmt);
    EmitStatementsWithIndent(stmt->statements);
}

void GeneratorImpl::EmitBlockHeader(const ast::BlockStatement* stmt) {
    if (!stmt->attributes.IsEmpty()) {
        Line() << "attrs: [";
        {
            ScopedIndent attrs(this);
            EmitAttributes(stmt->attributes);
        }
        Line() << "]";
    }
}

void GeneratorImpl::EmitStatement(const ast::Statement* stmt) {
    Switch(
        stmt,  //
        [&](const ast::AssignmentStatement* a) { EmitAssign(a); },
        [&](const ast::BlockStatement* b) { EmitBlock(b); },
        [&](const ast::BreakStatement* b) { EmitBreak(b); },
        [&](const ast::BreakIfStatement* b) { EmitBreakIf(b); },
        [&](const ast::CallStatement* c) { EmitCall(c->expr); },
        [&](const ast::CompoundAssignmentStatement* c) { EmitCompoundAssign(c); },
        [&](const ast::ContinueStatement* c) { EmitContinue(c); },
        [&](const ast::DiscardStatement* d) { EmitDiscard(d); },
        [&](const ast::IfStatement* i) { EmitIf(i); },
        [&](const ast::IncrementDecrementStatement* l) { EmitIncrementDecrement(l); },
        [&](const ast::LoopStatement* l) { EmitLoop(l); },
        [&](const ast::ForLoopStatement* l) { EmitForLoop(l); },
        [&](const ast::WhileStatement* l) { EmitWhile(l); },
        [&](const ast::ReturnStatement* r) { EmitReturn(r); },
        [&](const ast::ConstAssert* c) { EmitConstAssert(c); },
        [&](const ast::SwitchStatement* s) { EmitSwitch(s); },
        [&](const ast::VariableDeclStatement* v) { EmitVariable(v->variable); },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown statement type: " + std::string(stmt->TypeInfo().name));
        });
}

void GeneratorImpl::EmitStatements(utils::VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        EmitStatement(s);
    }
}

void GeneratorImpl::EmitStatementsWithIndent(utils::VectorRef<const ast::Statement*> stmts) {
    ScopedIndent si(this);
    EmitStatements(stmts);
}

void GeneratorImpl::EmitAssign(const ast::AssignmentStatement* stmt) {
    Line() << "AssignmentStatement [";
    {
        ScopedIndent as(this);
        Line() << "lhs: [";
        {
            ScopedIndent lhs(this);
            EmitExpression(stmt->lhs);
        }
        Line() << "]";
        Line() << "rhs: [";
        {
            ScopedIndent rhs(this);
            EmitExpression(stmt->rhs);
        }
        Line() << "]";
    }
    Line() << "]";
}

void GeneratorImpl::EmitBreak(const ast::BreakStatement*) {
    Line() << "BreakStatement []";
}

void GeneratorImpl::EmitBreakIf(const ast::BreakIfStatement* b) {
    Line() << "BreakIfStatement [";
    {
        ScopedIndent bis(this);
        EmitExpression(b->condition);
    }
    Line() << "]";
}

void GeneratorImpl::EmitCase(const ast::CaseStatement* stmt) {
    Line() << "CaseStatement [";
    {
        ScopedIndent cs(this);
        if (stmt->selectors.Length() == 1 && stmt->ContainsDefault()) {
            Line() << "selector: default";
            EmitBlockHeader(stmt->body);
        } else {
            Line() << "selectors: [";
            {
                ScopedIndent sels(this);
                for (auto* sel : stmt->selectors) {
                    if (sel->IsDefault()) {
                        Line() << "default []";
                    } else {
                        EmitExpression(sel->expr);
                    }
                }
            }
            Line() << "]";
            EmitBlockHeader(stmt->body);
        }
        EmitStatementsWithIndent(stmt->body->statements);
    }
    Line() << "]";
}

void GeneratorImpl::EmitCompoundAssign(const ast::CompoundAssignmentStatement* stmt) {
    Line() << "CompoundAssignmentStatement [";
    {
        ScopedIndent cas(this);
        Line() << "lhs: [";
        {
            ScopedIndent lhs(this);
            EmitExpression(stmt->lhs);
        }
        Line() << "]";

        Line() << "op: [";
        {
            ScopedIndent op(this);
            EmitBinaryOp(stmt->op);
        }
        Line() << "]";
        Line() << "rhs: [";
        {
            ScopedIndent rhs(this);

            EmitExpression(stmt->rhs);
        }
        Line() << "]";
    }
    Line() << "]";
}

void GeneratorImpl::EmitContinue(const ast::ContinueStatement*) {
    Line() << "ContinueStatement []";
}

void GeneratorImpl::EmitIf(const ast::IfStatement* stmt) {
    {
        Line() << "IfStatement [";
        {
            ScopedIndent ifs(this);
            Line() << "condition: [";
            {
                ScopedIndent cond(this);
                EmitExpression(stmt->condition);
            }
            Line() << "]";
            EmitBlockHeader(stmt->body);
        }
        Line() << "] ";
    }
    EmitStatementsWithIndent(stmt->body->statements);

    const ast::Statement* e = stmt->else_statement;
    while (e) {
        if (auto* elseif = e->As<ast::IfStatement>()) {
            {
                Line() << "Else IfStatement [";
                {
                    ScopedIndent ifs(this);
                    Line() << "condition: [";
                    EmitExpression(elseif->condition);
                }
                Line() << "]";
                EmitBlockHeader(elseif->body);
            }
            Line() << "]";
            EmitStatementsWithIndent(elseif->body->statements);
            e = elseif->else_statement;
        } else {
            auto* body = e->As<ast::BlockStatement>();
            {
                Line() << "Else [";
                {
                    ScopedIndent els(this);
                    EmitBlockHeader(body);
                }
                Line() << "]";
            }
            EmitStatementsWithIndent(body->statements);
            break;
        }
    }
}

void GeneratorImpl::EmitIncrementDecrement(const ast::IncrementDecrementStatement* stmt) {
    Line() << "IncrementDecrementStatement [";
    {
        ScopedIndent ids(this);
        Line() << "expr: [";
        EmitExpression(stmt->lhs);
        Line() << "]";
        Line() << "dir: " << (stmt->increment ? "++" : "--");
    }
    Line() << "]";
}

void GeneratorImpl::EmitDiscard(const ast::DiscardStatement*) {
    Line() << "DiscardStatement []";
}

void GeneratorImpl::EmitLoop(const ast::LoopStatement* stmt) {
    Line() << "LoopStatement [";
    {
        ScopedIndent ls(this);
        EmitStatements(stmt->body->statements);

        if (stmt->continuing && !stmt->continuing->Empty()) {
            Line() << "Continuing [";
            {
                ScopedIndent cont(this);
                EmitStatementsWithIndent(stmt->continuing->statements);
            }
            Line() << "]";
        }
    }
    Line() << "]";
}

void GeneratorImpl::EmitForLoop(const ast::ForLoopStatement* stmt) {
    TextBuffer init_buf;
    if (auto* init = stmt->initializer) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &init_buf);
        EmitStatement(init);
    }

    TextBuffer cont_buf;
    if (auto* cont = stmt->continuing) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cont_buf);
        EmitStatement(cont);
    }

    Line() << "ForLoopStatement [";
    {
        ScopedIndent fs(this);

        Line() << "initializer: [";
        {
            ScopedIndent init(this);
            switch (init_buf.lines.size()) {
                case 0:  // No initializer
                    break;
                case 1:  // Single line initializer statement
                    Line() << utils::TrimSuffix(init_buf.lines[0].content, ";");
                    break;
                default:  // Block initializer statement
                    for (size_t i = 1; i < init_buf.lines.size(); i++) {
                        // Indent all by the first line
                        init_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    Line() << utils::TrimSuffix(init_buf.String(), "\n");
                    break;
            }
        }
        Line() << "]";
        Line() << "condition: [";
        {
            ScopedIndent con(this);
            if (auto* cond = stmt->condition) {
                EmitExpression(cond);
            }
        }

        Line() << "]";
        Line() << "continuing: [";
        {
            ScopedIndent cont(this);
            switch (cont_buf.lines.size()) {
                case 0:  // No continuing
                    break;
                case 1:  // Single line continuing statement
                    Line() << utils::TrimSuffix(cont_buf.lines[0].content, ";");
                    break;
                default:  // Block continuing statement
                    for (size_t i = 1; i < cont_buf.lines.size(); i++) {
                        // Indent all by the first line
                        cont_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    Line() << utils::TrimSuffix(cont_buf.String(), "\n");
                    break;
            }
        }
        EmitBlockHeader(stmt->body);
        EmitStatementsWithIndent(stmt->body->statements);
    }
    Line() << "]";
}

void GeneratorImpl::EmitWhile(const ast::WhileStatement* stmt) {
    Line() << "WhileStatement [";
    {
        ScopedIndent ws(this);
        EmitExpression(stmt->condition);
        EmitBlockHeader(stmt->body);
        EmitStatementsWithIndent(stmt->body->statements);
    }
    Line() << "]";
}

void GeneratorImpl::EmitReturn(const ast::ReturnStatement* stmt) {
    Line() << "ReturnStatement [";
    {
        ScopedIndent ret(this);
        if (stmt->value) {
            EmitExpression(stmt->value);
        }
    }
    Line() << "]";
}

void GeneratorImpl::EmitConstAssert(const ast::ConstAssert* stmt) {
    Line() << "ConstAssert [";
    {
        ScopedIndent ca(this);
        EmitExpression(stmt->condition);
    }
    Line() << "]";
}

void GeneratorImpl::EmitSwitch(const ast::SwitchStatement* stmt) {
    Line() << "SwitchStatement [";
    {
        ScopedIndent ss(this);
        Line() << "condition: [";
        {
            ScopedIndent cond(this);
            EmitExpression(stmt->condition);
        }
        Line() << "]";

        {
            ScopedIndent si(this);
            for (auto* s : stmt->body) {
                EmitCase(s);
            }
        }
    }
    Line() << "]";
}

}  // namespace tint::writer::syntax_tree
