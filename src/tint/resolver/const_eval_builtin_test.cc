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

#include "src/tint/resolver/const_eval_test.h"

#include "src/tint/utils/result.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

// Bring in std::ostream& operator<<(std::ostream& o, const Types& types)
using resolver::operator<<;

struct Case {
    Case(utils::VectorRef<Types> in_args, Types expected_value)
        : args(std::move(in_args)), expected(Success{std::move(expected_value), false, false}) {}

    Case(utils::VectorRef<Types> in_args, const char* expected_err)
        : args(std::move(in_args)), expected(Failure{expected_err}) {}

    /// Expected value may be positive or negative
    Case& PosOrNeg() {
        Success s = expected.Get();
        s.pos_or_neg = true;
        expected = s;
        return *this;
    }

    /// Expected value should be compared using FLOAT_EQ instead of EQ
    Case& FloatComp() {
        Success s = expected.Get();
        s.float_compare = true;
        expected = s;
        return *this;
    }

    struct Success {
        Types value;
        bool pos_or_neg = false;
        bool float_compare = false;
    };
    struct Failure {
        const char* error = nullptr;
    };

    utils::Vector<Types, 8> args;
    utils::Result<Success, Failure> expected;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << "args: ";
    for (auto& a : c.args) {
        o << a << ", ";
    }
    o << "expected: ";
    if (c.expected) {
        auto s = c.expected.Get();
        o << s.value << ", pos_or_neg: " << s.pos_or_neg;
    } else {
        o << "[ERROR: " << c.expected.Failure().error << "]";
    }
    return o;
}

using ScalarTypes = std::variant<AInt, AFloat, u32, i32, f32, f16>;

/// Creates a Case with Values for args and result
static Case C(std::initializer_list<Types> args, Types result) {
    return Case{utils::Vector<Types, 8>{args}, std::move(result)};
}

/// Convenience overload that creates a Case with just scalars
static Case C(std::initializer_list<ScalarTypes> sargs, ScalarTypes sresult) {
    utils::Vector<Types, 8> args;
    for (auto& sa : sargs) {
        std::visit([&](auto&& v) { return args.Push(Val(v)); }, sa);
    }
    Types result = Val(0_a);
    std::visit([&](auto&& v) { result = Val(v); }, sresult);
    return Case{std::move(args), std::move(result)};
}

/// Creates a Case with Values for args and expected error
static Case E(std::initializer_list<Types> args, const char* err) {
    return Case{utils::Vector<Types, 8>{args}, err};
}

/// Convenience overload that creates an expected-error Case with just scalars
static Case E(std::initializer_list<ScalarTypes> sargs, const char* err) {
    utils::Vector<Types, 8> args;
    for (auto& sa : sargs) {
        std::visit([&](auto&& v) { return args.Push(Val(v)); }, sa);
    }
    return Case{std::move(args), err};
}

using ResolverConstEvalBuiltinTest = ResolverTestWithParam<std::tuple<sem::BuiltinType, Case>>;

TEST_P(ResolverConstEvalBuiltinTest, Test) {
    Enable(ast::Extension::kF16);

    auto builtin = std::get<0>(GetParam());
    auto& c = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> args;
    for (auto& a : c.args) {
        std::visit([&](auto&& v) { args.Push(v.Expr(*this)); }, a);
    }

    auto* expr = Call(Source{{12, 34}}, sem::str(builtin), std::move(args));

    GlobalConst("C", expr);

    if (c.expected) {
        auto expected = c.expected.Get();

        auto* expected_expr = ToValueBase(expected.value)->Expr(*this);
        GlobalConst("E", expected_expr);

        ASSERT_TRUE(r()->Resolve()) << r()->error();

        auto* sem = Sem().Get(expr);
        ASSERT_NE(sem, nullptr);
        const sem::Constant* value = sem->ConstantValue();
        ASSERT_NE(value, nullptr);
        EXPECT_TYPE(value->Type(), sem->Type());

        auto* expected_sem = Sem().Get(expected_expr);
        const sem::Constant* expected_value = expected_sem->ConstantValue();
        ASSERT_NE(expected_value, nullptr);
        EXPECT_TYPE(expected_value->Type(), expected_sem->Type());

        // @TODO(amaiorano): Rewrite using ScalarArgsFrom()
        ForEachElemPair(value, expected_value, [&](const sem::Constant* a, const sem::Constant* b) {
            std::visit(
                [&](auto&& ct_expected) {
                    using T = typename std::decay_t<decltype(ct_expected)>::ElementType;

                    auto v = a->As<T>();
                    auto e = b->As<T>();
                    if constexpr (std::is_same_v<bool, T>) {
                        EXPECT_EQ(v, e);
                    } else if constexpr (IsFloatingPoint<T>) {
                        if (std::isnan(e)) {
                            EXPECT_TRUE(std::isnan(v));
                        } else {
                            auto vf = (expected.pos_or_neg ? Abs(v) : v);
                            if (expected.float_compare) {
                                EXPECT_FLOAT_EQ(vf, e);
                            } else {
                                EXPECT_EQ(vf, e);
                            }
                        }
                    } else {
                        EXPECT_EQ((expected.pos_or_neg ? Abs(v) : v), e);
                        // Check that the constant's integer doesn't contain unexpected
                        // data in the MSBs that are outside of the bit-width of T.
                        EXPECT_EQ(a->As<AInt>(), b->As<AInt>());
                    }
                },
                expected.value);

            return HasFailure() ? Action::kStop : Action::kContinue;
        });
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), c.expected.Failure().error);
    }
}

INSTANTIATE_TEST_SUITE_P(  //
    MixedAbstractArgs,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan2),
                     testing::ValuesIn(std::vector{
                         C({0_a, -0.0_a}, kPi<AFloat>),
                         C({1.0_a, 0_a}, kPiOver2<AFloat>),
                     })));

template <typename T, bool finite_only>
std::vector<Case> AbsCases() {
    std::vector<Case> cases = {
        C({T(0)}, T(0)),
        C({T(2.0)}, T(2.0)),
        C({T::Highest()}, T::Highest()),

        // Vector tests
        C({Vec(T(2.0), T::Highest())}, Vec(T(2.0), T::Highest())),
    };

    ConcatIntoIf<IsSignedIntegral<T>>(
        cases,
        std::vector<Case>{
            C({Negate(T(0))}, T(0)),
            C({Negate(T(2.0))}, T(2.0)),
            // If e is signed and is the largest negative, the result is e
            C({T::Lowest()}, T::Lowest()),

            // 1 more then min i32
            C({Negate(T(2147483647))}, T(2147483647)),

            C({Vec(T(0), Negate(T(0)))}, Vec(T(0), T(0))),
            C({Vec(Negate(T(2.0)), T(2.0), T::Highest())}, Vec(T(2.0), T(2.0), T::Highest())),
        });

    ConcatIntoIf<!finite_only>(cases, std::vector<Case>{
                                          C({Negate(T::Inf())}, T::Inf()),
                                          C({T::Inf()}, T::Inf()),
                                          C({T::NaN()}, T::NaN()),
                                          C({Vec(Negate(T::Inf()), T::Inf(), T::NaN())},
                                            Vec(T::Inf(), T::Inf(), T::NaN())),
                                      });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Abs,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAbs),
                     testing::ValuesIn(Concat(AbsCases<AInt, false>(),  //
                                              AbsCases<i32, false>(),
                                              AbsCases<u32, false>(),
                                              AbsCases<AFloat, true>(),
                                              AbsCases<f32, false>(),
                                              AbsCases<f16, false>()))));

static std::vector<Case> AllCases() {
    return {
        C({Val(true)}, Val(true)),
        C({Val(false)}, Val(false)),

        C({Vec(true, true)}, Val(true)),
        C({Vec(true, false)}, Val(false)),
        C({Vec(false, true)}, Val(false)),
        C({Vec(false, false)}, Val(false)),

        C({Vec(true, true, true)}, Val(true)),
        C({Vec(false, true, true)}, Val(false)),
        C({Vec(true, false, true)}, Val(false)),
        C({Vec(true, true, false)}, Val(false)),
        C({Vec(false, false, false)}, Val(false)),

        C({Vec(true, true, true, true)}, Val(true)),
        C({Vec(false, true, true, true)}, Val(false)),
        C({Vec(true, false, true, true)}, Val(false)),
        C({Vec(true, true, false, true)}, Val(false)),
        C({Vec(true, true, true, false)}, Val(false)),
        C({Vec(false, false, false, false)}, Val(false)),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    All,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAll), testing::ValuesIn(AllCases())));

static std::vector<Case> AnyCases() {
    return {
        C({Val(true)}, Val(true)),
        C({Val(false)}, Val(false)),

        C({Vec(true, true)}, Val(true)),
        C({Vec(true, false)}, Val(true)),
        C({Vec(false, true)}, Val(true)),
        C({Vec(false, false)}, Val(false)),

        C({Vec(true, true, true)}, Val(true)),
        C({Vec(false, true, true)}, Val(true)),
        C({Vec(true, false, true)}, Val(true)),
        C({Vec(true, true, false)}, Val(true)),
        C({Vec(false, false, false)}, Val(false)),

        C({Vec(true, true, true, true)}, Val(true)),
        C({Vec(false, true, true, true)}, Val(true)),
        C({Vec(true, false, true, true)}, Val(true)),
        C({Vec(true, true, false, true)}, Val(true)),
        C({Vec(true, true, true, false)}, Val(true)),
        C({Vec(false, false, false, false)}, Val(false)),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Any,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAny), testing::ValuesIn(AnyCases())));

template <typename T, bool finite_only>
std::vector<Case> Atan2Cases() {
    std::vector<Case> cases = {
        // If y is +/-0 and x is negative or -0, +/-PI is returned
        C({T(0.0), -T(0.0)}, kPi<T>).PosOrNeg().FloatComp(),

        // If y is +/-0 and x is positive or +0, +/-0 is returned
        C({T(0.0), T(0.0)}, T(0.0)).PosOrNeg(),

        // If x is +/-0 and y is negative, -PI/2 is returned
        C({-T(1.0), T(0.0)}, -kPiOver2<T>).FloatComp(),  //
        C({-T(1.0), -T(0.0)}, -kPiOver2<T>).FloatComp(),

        // If x is +/-0 and y is positive, +PI/2 is returned
        C({T(1.0), T(0.0)}, kPiOver2<T>).FloatComp(),  //
        C({T(1.0), -T(0.0)}, kPiOver2<T>).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(0.0)), Vec(-T(0.0), T(0.0))}, Vec(kPi<T>, T(0.0))).PosOrNeg().FloatComp(),
        C({Vec(-T(1.0), -T(1.0)), Vec(T(0.0), -T(0.0))}, Vec(-kPiOver2<T>, -kPiOver2<T>))
            .FloatComp(),
        C({Vec(T(1.0), T(1.0)), Vec(T(0.0), -T(0.0))}, Vec(kPiOver2<T>, kPiOver2<T>)).FloatComp(),
    };

    ConcatIntoIf<!finite_only>(  //
        cases, std::vector<Case>{
                   // If y is +/-INF and x is finite, +/-PI/2 is returned
                   C({T::Inf(), T(0.0)}, kPiOver2<T>).PosOrNeg().FloatComp(),
                   C({-T::Inf(), T(0.0)}, kPiOver2<T>).PosOrNeg().FloatComp(),

                   // If y is +/-INF and x is -INF, +/-3PI/4 is returned
                   C({T::Inf(), -T::Inf()}, k3PiOver4<T>).PosOrNeg().FloatComp(),
                   C({-T::Inf(), -T::Inf()}, k3PiOver4<T>).PosOrNeg().FloatComp(),

                   // If y is +/-INF and x is +INF, +/-PI/4 is returned
                   C({T::Inf(), T::Inf()}, kPiOver4<T>).PosOrNeg().FloatComp(),
                   C({-T::Inf(), T::Inf()}, kPiOver4<T>).PosOrNeg().FloatComp(),

                   // If x is -INF and y is finite and positive, +PI is returned
                   C({T(0.0), -T::Inf()}, kPi<T>).FloatComp(),

                   // If x is -INF and y is finite and negative, -PI is returned
                   C({-T(0.0), -T::Inf()}, -kPi<T>).FloatComp(),

                   // If x is +INF and y is finite and positive, +0 is returned
                   C({T(0.0), T::Inf()}, T(0.0)),

                   // If x is +INF and y is finite and negative, -0 is returned
                   C({-T(0.0), T::Inf()}, -T(0.0)),

                   // If either x is NaN or y is NaN, NaN is returned
                   C({T::NaN(), T(0.0)}, T::NaN()),
                   C({T(0.0), T::NaN()}, T::NaN()),
                   C({T::NaN(), T::NaN()}, T::NaN()),

                   // Vector tests
                   C({Vec(T::Inf(), -T::Inf(), T::Inf(), -T::Inf()),  //
                      Vec(T(0.0), T(0.0), -T::Inf(), -T::Inf())},     //
                     Vec(kPiOver2<T>, kPiOver2<T>, k3PiOver4<T>, k3PiOver4<T>))
                       .PosOrNeg()
                       .FloatComp(),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Atan2,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan2),
                     testing::ValuesIn(Concat(Atan2Cases<AFloat, true>(),  //
                                              Atan2Cases<f32, false>(),
                                              Atan2Cases<f16, false>()))));

template <typename T, bool finite_only>
std::vector<Case> AtanCases() {
    std::vector<Case> cases = {
        C({T(1.0)}, kPiOver4<T>).FloatComp(),
        C({-T(1.0)}, -kPiOver4<T>).FloatComp(),

        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)).PosOrNeg(),

        // Vector tests
        C({Vec(T(0.0), T(1.0), -T(1.0))}, Vec(T(0.0), kPiOver4<T>, -kPiOver4<T>)).FloatComp(),
    };

    ConcatIntoIf<!finite_only>(  //
        cases, std::vector<Case>{
                   // If i is +/-INF, +/-PI/2 is returned
                   C({T::Inf()}, kPiOver2<T>).PosOrNeg().FloatComp(),
                   C({-T::Inf()}, -kPiOver2<T>).FloatComp(),

                   // If i is NaN, NaN is returned
                   C({T::NaN()}, T::NaN()),

                   // Vector tests
                   C({Vec(T::Inf(), -T::Inf(), T::Inf(), -T::Inf())},  //
                     Vec(kPiOver2<T>, -kPiOver2<T>, kPiOver2<T>, -kPiOver2<T>))
                       .FloatComp(),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Atan,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan),
                     testing::ValuesIn(Concat(AtanCases<AFloat, true>(),  //
                                              AtanCases<f32, false>(),
                                              AtanCases<f16, false>()))));

template <typename T, bool finite_only>
std::vector<Case> AtanhCases() {
    std::vector<Case> cases = {
        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)).PosOrNeg(),

        C({T(0.9)}, T(1.4722193)).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(0.9), -T(0.9))}, Vec(T(0.0), T(1.4722193), -T(1.4722193))).FloatComp(),
    };

    ConcatIntoIf<finite_only>(  //
        cases,
        std::vector<Case>{
            E({1.1_a},
              "12:34 error: atanh must be called with a value in the range (-1 .. 1) (exclusive)"),
            E({-1.1_a},
              "12:34 error: atanh must be called with a value in the range (-1 .. 1) (exclusive)"),
            E({T::Inf()},
              "12:34 error: atanh must be called with a value in the range (-1 .. 1) (exclusive)"),
            E({-T::Inf()},
              "12:34 error: atanh must be called with a value in the range (-1 .. 1) (exclusive)"),
        });

    ConcatIntoIf<!finite_only>(  //
        cases, std::vector<Case>{
                   // If i is NaN, NaN is returned
                   C({T::NaN()}, T::NaN()),

                   // Vector tests
                   C({Vec(T::NaN(), T::NaN())}, Vec(T::NaN(), T::NaN())).FloatComp(),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Atanh,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtanh),
                     testing::ValuesIn(Concat(AtanhCases<AFloat, true>(),  //
                                              AtanhCases<f32, false>(),
                                              AtanhCases<f16, false>()))));

template <typename T, bool finite_only>
std::vector<Case> AcosCases() {
    std::vector<Case> cases = {
        // If i is +/-0, +/-0 is returned
        C({T(0.87758256189)}, T(0.5)).FloatComp(),

        C({T(1.0)}, T(0.0)),
        C({-T(1.0)}, kPi<T>).FloatComp(),

        // Vector tests
        C({Vec(T(1.0), -T(1.0))}, Vec(T(0), kPi<T>)).FloatComp(),
    };

    ConcatIntoIf<finite_only>(  //
        cases,
        std::vector<Case>{
            E({1.1_a},
              "12:34 error: acos must be called with a value in the range [-1 .. 1] (inclusive)"),
            E({-1.1_a},
              "12:34 error: acos must be called with a value in the range [-1 .. 1] (inclusive)"),
            E({T::Inf()},
              "12:34 error: acos must be called with a value in the range [-1 .. 1] (inclusive)"),
            E({-T::Inf()},
              "12:34 error: acos must be called with a value in the range [-1 .. 1] (inclusive)"),
        });

    ConcatIntoIf<!finite_only>(  //
        cases, std::vector<Case>{
                   // If i is NaN, NaN is returned
                   C({T::NaN()}, T::NaN()),

                   // Vector tests
                   C({Vec(T::NaN(), T::NaN())}, Vec(T::NaN(), T::NaN())),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Acos,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAcos),
                     testing::ValuesIn(Concat(AcosCases<AFloat, true>(),  //
                                              AcosCases<f32, false>(),
                                              AcosCases<f16, false>()))));

template <typename T, bool finite_only>
std::vector<Case> AsinCases() {
    std::vector<Case> cases = {
        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)),
        C({-T(0.0)}, -T(0.0)),

        C({T(1.0)}, kPiOver2<T>).FloatComp(),
        C({-T(1.0)}, -kPiOver2<T>).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(1.0), -T(1.0))}, Vec(T(0.0), kPiOver2<T>, -kPiOver2<T>)).FloatComp(),
    };

    ConcatIntoIf<finite_only>(  //
        cases,
        std::vector<Case>{
            E({1.1_a},
              "12:34 error: asin must be called with a value in the range [-1 .. 1] (inclusive)"),
            E({-1.1_a},
              "12:34 error: asin must be called with a value in the range [-1 .. 1] (inclusive)"),
            E({T::Inf()},
              "12:34 error: asin must be called with a value in the range [-1 .. 1] (inclusive)"),
            E({-T::Inf()},
              "12:34 error: asin must be called with a value in the range [-1 .. 1] (inclusive)"),
        });

    ConcatIntoIf<!finite_only>(  //
        cases, std::vector<Case>{
                   // If i is NaN, NaN is returned
                   C({T::NaN()}, T::NaN()),

                   // Vector tests
                   C({Vec(T::NaN(), T::NaN())}, Vec(T::NaN(), T::NaN())).FloatComp(),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Asin,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAsin),
                     testing::ValuesIn(Concat(AsinCases<AFloat, true>(),  //
                                              AsinCases<f32, false>(),
                                              AsinCases<f16, false>()))));

template <typename T, bool finite_only>
std::vector<Case> AsinhCases() {
    std::vector<Case> cases = {
        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)),
        C({-T(0.0)}, -T(0.0)),

        C({T(0.9)}, T(0.80886693565278)).FloatComp(),
        C({-T(2.0)}, -T(1.4436354751788)).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(0.9), -T(2.0))},  //
          Vec(T(0.0), T(0.8088669356278), -T(1.4436354751788)))
            .FloatComp(),
    };

    ConcatIntoIf<!finite_only>(  //
        cases, std::vector<Case>{
                   // If i is +/- INF, +/-INF is returned
                   C({T::Inf()}, T::Inf()),
                   C({-T::Inf()}, -T::Inf()),

                   // If i is NaN, NaN is returned
                   C({T::NaN()}, T::NaN()),

                   // Vector tests
                   C({Vec(T::Inf(), T::NaN(), -T::Inf())},  //
                     Vec(T::Inf(), T::NaN(), -T::Inf())),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Asinh,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAsinh),
                     testing::ValuesIn(Concat(AsinhCases<AFloat, true>(),  //
                                              AsinhCases<f32, false>(),
                                              AsinhCases<f16, false>()))));

template <typename T, bool finite_only>
std::vector<Case> CeilCases() {
    std::vector<Case> cases = {
        C({T(0)}, T(0)),
        C({-T(0)}, -T(0)),
        C({-T(1.5)}, -T(1.0)),
        C({T(1.5)}, T(2.0)),
        C({T::Lowest()}, T::Lowest()),
        C({T::Highest()}, T::Highest()),

        C({Vec(T(0), T(1.5), -T(1.5))}, Vec(T(0), T(2.0), -T(1.0))),
    };

    ConcatIntoIf<!finite_only>(
        cases, std::vector<Case>{
                   C({-T::Inf()}, -T::Inf()),
                   C({T::Inf()}, T::Inf()),
                   C({T::NaN()}, T::NaN()),
                   C({Vec(-T::Inf(), T::Inf(), T::NaN())}, Vec(-T::Inf(), T::Inf(), T::NaN())),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Ceil,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kCeil),
                     testing::ValuesIn(Concat(CeilCases<AFloat, true>(),
                                              CeilCases<f32, false>(),
                                              CeilCases<f16, false>()))));

template <typename T>
std::vector<Case> ClampCases() {
    return {
        C({T(0), T(0), T(0)}, T(0)),
        C({T(0), T(42), T::Highest()}, T(42)),
        C({T::Lowest(), T(0), T(42)}, T(0)),
        C({T(0), T::Lowest(), T::Highest()}, T(0)),
        C({T(0), T::Highest(), T::Lowest()}, T::Lowest()),
        C({T::Highest(), T::Highest(), T::Highest()}, T::Highest()),
        C({T::Lowest(), T::Lowest(), T::Lowest()}, T::Lowest()),
        C({T::Highest(), T::Lowest(), T::Highest()}, T::Highest()),
        C({T::Lowest(), T::Lowest(), T::Highest()}, T::Lowest()),

        // Vector tests
        C({Vec(T(0), T(0)),                         //
           Vec(T(0), T(42)),                        //
           Vec(T(0), T::Highest())},                //
          Vec(T(0), T(42))),                        //
        C({Vec(T::Lowest(), T(0), T(0)),            //
           Vec(T(0), T::Lowest(), T::Highest()),    //
           Vec(T(42), T::Highest(), T::Lowest())},  //
          Vec(T(0), T(0), T::Lowest())),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Clamp,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kClamp),
                     testing::ValuesIn(Concat(ClampCases<AInt>(),  //
                                              ClampCases<i32>(),
                                              ClampCases<u32>(),
                                              ClampCases<AFloat>(),
                                              ClampCases<f32>(),
                                              ClampCases<f16>()))));

template <typename T>
std::vector<Case> CountLeadingZerosCases() {
    using B = BitValues<T>;
    return {
        C({B::Lsh(1, 31)}, T(0)),  //
        C({B::Lsh(1, 30)}, T(1)),  //
        C({B::Lsh(1, 29)}, T(2)),  //
        C({B::Lsh(1, 28)}, T(3)),
        //...
        C({B::Lsh(1, 3)}, T(28)),  //
        C({B::Lsh(1, 2)}, T(29)),  //
        C({B::Lsh(1, 1)}, T(30)),  //
        C({B::Lsh(1, 0)}, T(31)),

        C({T(0b1111'0000'1111'0000'1111'0000'1111'0000)}, T(0)),
        C({T(0b0111'1000'0111'1000'0111'1000'0111'1000)}, T(1)),
        C({T(0b0011'1100'0011'1100'0011'1100'0011'1100)}, T(2)),
        C({T(0b0001'1110'0001'1110'0001'1110'0001'1110)}, T(3)),
        //...
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0111)}, T(29)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0011)}, T(30)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0001)}, T(31)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0000)}, T(32)),

        // Same as above, but remove leading 0
        C({T(0b1111'1000'0111'1000'0111'1000'0111'1000)}, T(0)),
        C({T(0b1011'1100'0011'1100'0011'1100'0011'1100)}, T(0)),
        C({T(0b1001'1110'0001'1110'0001'1110'0001'1110)}, T(0)),
        //...
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0111)}, T(0)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0011)}, T(0)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0001)}, T(0)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0000)}, T(0)),

        // Vector tests
        C({Vec(B::Lsh(1, 31), B::Lsh(1, 30), B::Lsh(1, 29))}, Vec(T(0), T(1), T(2))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(29), T(30), T(31))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    CountLeadingZeros,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kCountLeadingZeros),
                     testing::ValuesIn(Concat(CountLeadingZerosCases<i32>(),  //
                                              CountLeadingZerosCases<u32>()))));

template <typename T>
std::vector<Case> CountTrailingZerosCases() {
    using B = BitValues<T>;
    return {
        C({B::Lsh(1, 31)}, T(31)),  //
        C({B::Lsh(1, 30)}, T(30)),  //
        C({B::Lsh(1, 29)}, T(29)),  //
        C({B::Lsh(1, 28)}, T(28)),
        //...
        C({B::Lsh(1, 3)}, T(3)),  //
        C({B::Lsh(1, 2)}, T(2)),  //
        C({B::Lsh(1, 1)}, T(1)),  //
        C({B::Lsh(1, 0)}, T(0)),

        C({T(0b0000'1111'0000'1111'0000'1111'0000'1111)}, T(0)),
        C({T(0b0001'1110'0001'1110'0001'1110'0001'1110)}, T(1)),
        C({T(0b0011'1100'0011'1100'0011'1100'0011'1100)}, T(2)),
        C({T(0b0111'1000'0111'1000'0111'1000'0111'1000)}, T(3)),
        //...
        C({T(0b1110'0000'0000'0000'0000'0000'0000'0000)}, T(29)),
        C({T(0b1100'0000'0000'0000'0000'0000'0000'0000)}, T(30)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0000)}, T(31)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0000)}, T(32)),

        //// Same as above, but remove trailing 0
        C({T(0b0001'1110'0001'1110'0001'1110'0001'1111)}, T(0)),
        C({T(0b0011'1100'0011'1100'0011'1100'0011'1101)}, T(0)),
        C({T(0b0111'1000'0111'1000'0111'1000'0111'1001)}, T(0)),
        //...
        C({T(0b1110'0000'0000'0000'0000'0000'0000'0001)}, T(0)),
        C({T(0b1100'0000'0000'0000'0000'0000'0000'0001)}, T(0)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0001)}, T(0)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0001)}, T(0)),

        // Vector tests
        C({Vec(B::Lsh(1, 31), B::Lsh(1, 30), B::Lsh(1, 29))}, Vec(T(31), T(30), T(29))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(2), T(1), T(0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    CountTrailingZeros,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kCountTrailingZeros),
                     testing::ValuesIn(Concat(CountTrailingZerosCases<i32>(),  //
                                              CountTrailingZerosCases<u32>()))));

template <typename T>
std::vector<Case> CountOneBitsCases() {
    using B = BitValues<T>;
    return {
        C({T(0)}, T(0)),  //

        C({B::Lsh(1, 31)}, T(1)),  //
        C({B::Lsh(1, 30)}, T(1)),  //
        C({B::Lsh(1, 29)}, T(1)),  //
        C({B::Lsh(1, 28)}, T(1)),
        //...
        C({B::Lsh(1, 3)}, T(1)),  //
        C({B::Lsh(1, 2)}, T(1)),  //
        C({B::Lsh(1, 1)}, T(1)),  //
        C({B::Lsh(1, 0)}, T(1)),

        C({T(0b1010'1010'1010'1010'1010'1010'1010'1010)}, T(16)),
        C({T(0b0000'1111'0000'1111'0000'1111'0000'1111)}, T(16)),
        C({T(0b0101'0000'0000'0000'0000'0000'0000'0101)}, T(4)),

        // Vector tests
        C({Vec(B::Lsh(1, 31), B::Lsh(1, 30), B::Lsh(1, 29))}, Vec(T(1), T(1), T(1))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(1), T(1), T(1))),

        C({Vec(T(0b1010'1010'1010'1010'1010'1010'1010'1010),
               T(0b0000'1111'0000'1111'0000'1111'0000'1111),
               T(0b0101'0000'0000'0000'0000'0000'0000'0101))},
          Vec(T(16), T(16), T(4))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    CountOneBits,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kCountOneBits),
                     testing::ValuesIn(Concat(CountOneBitsCases<i32>(),  //
                                              CountOneBitsCases<u32>()))));

template <typename T>
std::vector<Case> FirstLeadingBitCases() {
    using B = BitValues<T>;
    auto r = std::vector<Case>{
        // Both signed and unsigned return T(-1) for input 0
        C({T(0)}, T(-1)),

        C({B::Lsh(1, 30)}, T(30)),  //
        C({B::Lsh(1, 29)}, T(29)),  //
        C({B::Lsh(1, 28)}, T(28)),
        //...
        C({B::Lsh(1, 3)}, T(3)),  //
        C({B::Lsh(1, 2)}, T(2)),  //
        C({B::Lsh(1, 1)}, T(1)),  //
        C({B::Lsh(1, 0)}, T(0)),

        C({T(0b0000'0000'0100'1000'1000'1000'0000'0000)}, T(22)),
        C({T(0b0000'0000'0000'0100'1000'1000'0000'0000)}, T(18)),

        // Vector tests
        C({Vec(B::Lsh(1, 30), B::Lsh(1, 29), B::Lsh(1, 28))}, Vec(T(30), T(29), T(28))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(2), T(1), T(0))),
    };

    ConcatIntoIf<IsUnsignedIntegral<T>>(  //
        r, std::vector<Case>{
               C({B::Lsh(1, 31)}, T(31)),

               C({T(0b1111'1111'1111'1111'1111'1111'1111'1110)}, T(31)),
               C({T(0b1111'1111'1111'1111'1111'1111'1111'1100)}, T(31)),
               C({T(0b1111'1111'1111'1111'1111'1111'1111'1000)}, T(31)),
               //...
               C({T(0b1110'0000'0000'0000'0000'0000'0000'0000)}, T(31)),
               C({T(0b1100'0000'0000'0000'0000'0000'0000'0000)}, T(31)),
               C({T(0b1000'0000'0000'0000'0000'0000'0000'0000)}, T(31)),
           });

    ConcatIntoIf<IsSignedIntegral<T>>(  //
        r, std::vector<Case>{
               // Signed returns -1 for input -1
               C({T(-1)}, T(-1)),

               C({B::Lsh(1, 31)}, T(30)),

               C({T(0b1111'1111'1111'1111'1111'1111'1111'1110)}, T(0)),
               C({T(0b1111'1111'1111'1111'1111'1111'1111'1100)}, T(1)),
               C({T(0b1111'1111'1111'1111'1111'1111'1111'1000)}, T(2)),
               //...
               C({T(0b1110'0000'0000'0000'0000'0000'0000'0000)}, T(28)),
               C({T(0b1100'0000'0000'0000'0000'0000'0000'0000)}, T(29)),
               C({T(0b1000'0000'0000'0000'0000'0000'0000'0000)}, T(30)),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    FirstLeadingBit,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kFirstLeadingBit),
                     testing::ValuesIn(Concat(FirstLeadingBitCases<i32>(),  //
                                              FirstLeadingBitCases<u32>()))));

template <typename T>
std::vector<Case> FirstTrailingBitCases() {
    using B = BitValues<T>;
    auto r = std::vector<Case>{
        C({T(0)}, T(-1)),

        C({B::Lsh(1, 31)}, T(31)),  //
        C({B::Lsh(1, 30)}, T(30)),  //
        C({B::Lsh(1, 29)}, T(29)),  //
        C({B::Lsh(1, 28)}, T(28)),
        //...
        C({B::Lsh(1, 3)}, T(3)),  //
        C({B::Lsh(1, 2)}, T(2)),  //
        C({B::Lsh(1, 1)}, T(1)),  //
        C({B::Lsh(1, 0)}, T(0)),

        C({T(0b0000'0000'0100'1000'1000'1000'0000'0000)}, T(11)),
        C({T(0b0000'0100'1000'1000'1000'0000'0000'0000)}, T(15)),

        // Vector tests
        C({Vec(B::Lsh(1, 31), B::Lsh(1, 30), B::Lsh(1, 29))}, Vec(T(31), T(30), T(29))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(2), T(1), T(0))),
    };

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    FirstTrailingBit,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kFirstTrailingBit),
                     testing::ValuesIn(Concat(FirstTrailingBitCases<i32>(),  //
                                              FirstTrailingBitCases<u32>()))));

template <typename T, bool finite_only>
std::vector<Case> FloorCases() {
    std::vector<Case> cases = {
        C({T(0)}, T(0)),
        C({-T(0)}, -T(0)),
        C({-T(1.5)}, -T(2.0)),
        C({T(1.5)}, T(1.0)),
        C({T::Lowest()}, T::Lowest()),
        C({T::Highest()}, T::Highest()),

        C({Vec(T(0), T(1.5), -T(1.5))}, Vec(T(0), T(1.0), -T(2.0))),
    };

    ConcatIntoIf<!finite_only>(
        cases, std::vector<Case>{
                   C({-T::Inf()}, -T::Inf()),
                   C({T::Inf()}, T::Inf()),
                   C({T::NaN()}, T::NaN()),
                   C({Vec(-T::Inf(), T::Inf(), T::NaN())}, Vec(-T::Inf(), T::Inf(), T::NaN())),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Floor,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kFloor),
                     testing::ValuesIn(Concat(FloorCases<AFloat, true>(),
                                              FloorCases<f32, false>(),
                                              FloorCases<f16, false>()))));

template <typename T>
std::vector<Case> InsertBitsCases() {
    using UT = Number<std::make_unsigned_t<UnwrapNumber<T>>>;

    auto e = /* */ T(0b0101'1100'0011'1010'0101'1100'0011'1010);
    auto newbits = T{0b1010'0011'1100'0101'1010'0011'1100'0101};

    auto r = std::vector<Case>{
        // args: e, newbits, offset, count

        // If count is 0, result is e
        C({e, newbits, UT(0), UT(0)}, e),  //
        C({e, newbits, UT(1), UT(0)}, e),  //
        C({e, newbits, UT(2), UT(0)}, e),  //
        C({e, newbits, UT(3), UT(0)}, e),  //
        // ...
        C({e, newbits, UT(29), UT(0)}, e),  //
        C({e, newbits, UT(30), UT(0)}, e),  //
        C({e, newbits, UT(31), UT(0)}, e),

        // Copy 1 to 32 bits of newbits to e at offset 0
        C({e, newbits, UT(0), UT(1)}, T(0b0101'1100'0011'1010'0101'1100'0011'1011)),
        C({e, newbits, UT(0), UT(2)}, T(0b0101'1100'0011'1010'0101'1100'0011'1001)),
        C({e, newbits, UT(0), UT(3)}, T(0b0101'1100'0011'1010'0101'1100'0011'1101)),
        C({e, newbits, UT(0), UT(4)}, T(0b0101'1100'0011'1010'0101'1100'0011'0101)),
        C({e, newbits, UT(0), UT(5)}, T(0b0101'1100'0011'1010'0101'1100'0010'0101)),
        C({e, newbits, UT(0), UT(6)}, T(0b0101'1100'0011'1010'0101'1100'0000'0101)),
        // ...
        C({e, newbits, UT(0), UT(29)}, T(0b0100'0011'1100'0101'1010'0011'1100'0101)),
        C({e, newbits, UT(0), UT(30)}, T(0b0110'0011'1100'0101'1010'0011'1100'0101)),
        C({e, newbits, UT(0), UT(31)}, T(0b0010'0011'1100'0101'1010'0011'1100'0101)),
        C({e, newbits, UT(0), UT(32)}, T(0b1010'0011'1100'0101'1010'0011'1100'0101)),

        // Copy at varying offsets and counts
        C({e, newbits, UT(3), UT(8)}, T(0b0101'1100'0011'1010'0101'1110'0010'1010)),
        C({e, newbits, UT(8), UT(8)}, T(0b0101'1100'0011'1010'1100'0101'0011'1010)),
        C({e, newbits, UT(15), UT(1)}, T(0b0101'1100'0011'1010'1101'1100'0011'1010)),
        C({e, newbits, UT(16), UT(16)}, T(0b1010'0011'1100'0101'0101'1100'0011'1010)),

        // Vector tests
        C({Vec(T(0b1111'0000'1111'0000'1111'0000'1111'0000),  //
               T(0b0000'1111'0000'1111'0000'1111'0000'1111),  //
               T(0b1010'0101'1010'0101'1010'0101'1010'0101)),
           Vec(T(0b1111'1111'1111'1111'1111'1111'1111'1111),  //
               T(0b1111'1111'1111'1111'1111'1111'1111'1111),  //
               T(0b1111'1111'1111'1111'1111'1111'1111'1111)),
           Val(UT(3)), Val(UT(8))},
          Vec(T(0b1111'0000'1111'0000'1111'0111'1111'1000),  //
              T(0b0000'1111'0000'1111'0000'1111'1111'1111),  //
              T(0b1010'0101'1010'0101'1010'0111'1111'1101))),
    };

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    InsertBits,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kInsertBits),
                     testing::ValuesIn(Concat(InsertBitsCases<i32>(),  //
                                              InsertBitsCases<u32>()))));

using ResolverConstEvalBuiltinTest_InsertBits_InvalidOffsetAndCount =
    ResolverTestWithParam<std::tuple<size_t, size_t>>;
TEST_P(ResolverConstEvalBuiltinTest_InsertBits_InvalidOffsetAndCount, Test) {
    auto& p = GetParam();
    auto* expr = Call(Source{{12, 34}}, sem::str(sem::BuiltinType::kInsertBits), Expr(1_u),
                      Expr(1_u), Expr(u32(std::get<0>(p))), Expr(u32(std::get<1>(p))));
    GlobalConst("C", expr);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: 'offset + 'count' must be less than or equal to the bit width of 'e'");
}
INSTANTIATE_TEST_SUITE_P(InsertBits,
                         ResolverConstEvalBuiltinTest_InsertBits_InvalidOffsetAndCount,
                         testing::Values(                         //
                             std::make_tuple(33, 0),              //
                             std::make_tuple(34, 0),              //
                             std::make_tuple(1000, 0),            //
                             std::make_tuple(u32::Highest(), 0),  //
                             std::make_tuple(0, 33),              //
                             std::make_tuple(0, 34),              //
                             std::make_tuple(0, 1000),            //
                             std::make_tuple(0, u32::Highest()),  //
                             std::make_tuple(33, 33),             //
                             std::make_tuple(34, 34),             //
                             std::make_tuple(1000, 1000),         //
                             std::make_tuple(u32::Highest(), u32::Highest())));

template <typename T>
std::vector<Case> ExtractBitsCases() {
    using UT = Number<std::make_unsigned_t<UnwrapNumber<T>>>;

    // If T is signed, fills most significant bits of `val` with 1s
    auto set_msbs_if_signed = [](T val) {
        if constexpr (IsSignedIntegral<T>) {
            T result = T(~0);
            for (size_t b = 0; val; ++b) {
                if ((val & 1) == 0) {
                    result = result & ~(1 << b);  // Clear bit b
                }
                val = val >> 1;
            }
            return result;
        } else {
            return val;
        }
    };

    auto e = T(0b10100011110001011010001111000101);
    auto f = T(0b01010101010101010101010101010101);
    auto g = T(0b11111010001111000101101000111100);

    auto r = std::vector<Case>{
        // args: e, offset, count

        // If count is 0, result is 0
        C({e, UT(0), UT(0)}, T(0)),  //
        C({e, UT(1), UT(0)}, T(0)),  //
        C({e, UT(2), UT(0)}, T(0)),  //
        C({e, UT(3), UT(0)}, T(0)),
        // ...
        C({e, UT(29), UT(0)}, T(0)),  //
        C({e, UT(30), UT(0)}, T(0)),  //
        C({e, UT(31), UT(0)}, T(0)),

        // Extract at offset 0, varying counts
        C({e, UT(0), UT(1)}, set_msbs_if_signed(T(0b1))),    //
        C({e, UT(0), UT(2)}, T(0b01)),                       //
        C({e, UT(0), UT(3)}, set_msbs_if_signed(T(0b101))),  //
        C({e, UT(0), UT(4)}, T(0b0101)),                     //
        C({e, UT(0), UT(5)}, T(0b00101)),                    //
        C({e, UT(0), UT(6)}, T(0b000101)),                   //
        // ...
        C({e, UT(0), UT(28)}, T(0b0011110001011010001111000101)),                        //
        C({e, UT(0), UT(29)}, T(0b00011110001011010001111000101)),                       //
        C({e, UT(0), UT(30)}, set_msbs_if_signed(T(0b100011110001011010001111000101))),  //
        C({e, UT(0), UT(31)}, T(0b0100011110001011010001111000101)),                     //
        C({e, UT(0), UT(32)}, T(0b10100011110001011010001111000101)),                    //

        // Extract at varying offsets and counts
        C({e, UT(0), UT(1)}, set_msbs_if_signed(T(0b1))),                   //
        C({e, UT(31), UT(1)}, set_msbs_if_signed(T(0b1))),                  //
        C({e, UT(3), UT(5)}, set_msbs_if_signed(T(0b11000))),               //
        C({e, UT(4), UT(7)}, T(0b0111100)),                                 //
        C({e, UT(10), UT(16)}, set_msbs_if_signed(T(0b1111000101101000))),  //
        C({e, UT(10), UT(22)}, set_msbs_if_signed(T(0b1010001111000101101000))),

        // Vector tests
        C({Vec(e, f, g),                          //
           Val(UT(5)), Val(UT(8))},               //
          Vec(T(0b00011110),                      //
              set_msbs_if_signed(T(0b10101010)),  //
              set_msbs_if_signed(T(0b11010001)))),
    };

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    ExtractBits,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kExtractBits),
                     testing::ValuesIn(Concat(ExtractBitsCases<i32>(),  //
                                              ExtractBitsCases<u32>()))));

using ResolverConstEvalBuiltinTest_ExtractBits_InvalidOffsetAndCount =
    ResolverTestWithParam<std::tuple<size_t, size_t>>;
TEST_P(ResolverConstEvalBuiltinTest_ExtractBits_InvalidOffsetAndCount, Test) {
    auto& p = GetParam();
    auto* expr = Call(Source{{12, 34}}, sem::str(sem::BuiltinType::kExtractBits), Expr(1_u),
                      Expr(u32(std::get<0>(p))), Expr(u32(std::get<1>(p))));
    GlobalConst("C", expr);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: 'offset + 'count' must be less than or equal to the bit width of 'e'");
}
INSTANTIATE_TEST_SUITE_P(ExtractBits,
                         ResolverConstEvalBuiltinTest_ExtractBits_InvalidOffsetAndCount,
                         testing::Values(                         //
                             std::make_tuple(33, 0),              //
                             std::make_tuple(34, 0),              //
                             std::make_tuple(1000, 0),            //
                             std::make_tuple(u32::Highest(), 0),  //
                             std::make_tuple(0, 33),              //
                             std::make_tuple(0, 34),              //
                             std::make_tuple(0, 1000),            //
                             std::make_tuple(0, u32::Highest()),  //
                             std::make_tuple(33, 33),             //
                             std::make_tuple(34, 34),             //
                             std::make_tuple(1000, 1000),         //
                             std::make_tuple(u32::Highest(), u32::Highest())));

template <typename T>
std::vector<Case> ReverseBitsCases() {
    using B = BitValues<T>;
    return {
        C({T(0)}, T(0)),

        C({B::Lsh(1, 0)}, B::Lsh(1, 31)),  //
        C({B::Lsh(1, 1)}, B::Lsh(1, 30)),  //
        C({B::Lsh(1, 2)}, B::Lsh(1, 29)),  //
        C({B::Lsh(1, 3)}, B::Lsh(1, 28)),  //
        C({B::Lsh(1, 4)}, B::Lsh(1, 27)),  //
        //...
        C({B::Lsh(1, 27)}, B::Lsh(1, 4)),  //
        C({B::Lsh(1, 28)}, B::Lsh(1, 3)),  //
        C({B::Lsh(1, 29)}, B::Lsh(1, 2)),  //
        C({B::Lsh(1, 30)}, B::Lsh(1, 1)),  //
        C({B::Lsh(1, 31)}, B::Lsh(1, 0)),  //

        C({/**/ T(0b00010001000100010000000000000000)},
          /* */ T(0b00000000000000001000100010001000)),

        C({/**/ T(0b00011000000110000000000000000000)},
          /* */ T(0b00000000000000000001100000011000)),

        C({/**/ T(0b00000100000000001111111111111111)},
          /* */ T(0b11111111111111110000000000100000)),

        C({/**/ T(0b10010101111000110000011111101010)},
          /* */ T(0b01010111111000001100011110101001)),

        // Vector tests
        C({/**/ Vec(T(0b00010001000100010000000000000000),  //
                    T(0b00011000000110000000000000000000),  //
                    T(0b00000000000000001111111111111111))},
          /* */ Vec(T(0b00000000000000001000100010001000),  //
                    T(0b00000000000000000001100000011000),  //
                    T(0b11111111111111110000000000000000))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    ReverseBits,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kReverseBits),
                     testing::ValuesIn(Concat(ReverseBitsCases<i32>(),  //
                                              ReverseBitsCases<u32>()))));

template <typename T>
std::vector<Case> SaturateCases() {
    return {
        C({T(0)}, T(0)),
        C({T(1)}, T(1)),
        C({T::Lowest()}, T(0)),
        C({T::Highest()}, T(1)),

        // Vector tests
        C({Vec(T(0), T(0))},                       //
          Vec(T(0), T(0))),                        //
        C({Vec(T(1), T(1))},                       //
          Vec(T(1), T(1))),                        //
        C({Vec(T::Lowest(), T(0), T::Highest())},  //
          Vec(T(0), T(0), T(1))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Saturate,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kSaturate),
                     testing::ValuesIn(Concat(SaturateCases<AFloat>(),  //
                                              SaturateCases<f32>(),
                                              SaturateCases<f16>()))));

template <typename T>
std::vector<Case> SelectCases() {
    return {
        C({Val(T{1}), Val(T{2}), Val(false)}, Val(T{1})),
        C({Val(T{1}), Val(T{2}), Val(true)}, Val(T{2})),

        C({Val(T{2}), Val(T{1}), Val(false)}, Val(T{2})),
        C({Val(T{2}), Val(T{1}), Val(true)}, Val(T{1})),

        C({Vec(T{1}, T{2}), Vec(T{3}, T{4}), Vec(false, false)}, Vec(T{1}, T{2})),
        C({Vec(T{1}, T{2}), Vec(T{3}, T{4}), Vec(false, true)}, Vec(T{1}, T{4})),
        C({Vec(T{1}, T{2}), Vec(T{3}, T{4}), Vec(true, false)}, Vec(T{3}, T{2})),
        C({Vec(T{1}, T{2}), Vec(T{3}, T{4}), Vec(true, true)}, Vec(T{3}, T{4})),

        C({Vec(T{1}, T{1}, T{2}, T{2}),     //
           Vec(T{2}, T{2}, T{1}, T{1}),     //
           Vec(false, true, false, true)},  //
          Vec(T{1}, T{2}, T{2}, T{1})),     //
    };
}
static std::vector<Case> SelectBoolCases() {
    return {
        C({Val(true), Val(false), Val(false)}, Val(true)),
        C({Val(true), Val(false), Val(true)}, Val(false)),

        C({Val(false), Val(true), Val(true)}, Val(true)),
        C({Val(false), Val(true), Val(false)}, Val(false)),

        C({Vec(true, true, false, false),   //
           Vec(false, false, true, true),   //
           Vec(false, true, true, false)},  //
          Vec(true, false, true, false)),   //
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Select,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kSelect),
                     testing::ValuesIn(Concat(SelectCases<AInt>(),  //
                                              SelectCases<i32>(),
                                              SelectCases<u32>(),
                                              SelectCases<AFloat>(),
                                              SelectCases<f32>(),
                                              SelectCases<f16>(),
                                              SelectBoolCases()))));

template <typename T>
std::vector<Case> SignCases() {
    return {
        C({-T(1)}, -T(1)),
        C({-T(0.5)}, -T(1)),
        C({T(0)}, T(0)),
        C({-T(0)}, T(0)),
        C({T(0.5)}, T(1)),
        C({T(1)}, T(1)),

        C({T::Highest()}, T(1.0)),
        C({T::Lowest()}, -T(1.0)),

        // Vector tests
        C({Vec(-T(0.5), T(0), T(0.5))}, Vec(-T(1.0), T(0.0), T(1.0))),
        C({Vec(T::Highest(), T::Lowest())}, Vec(T(1.0), -T(1.0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Sign,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kSign),
                     testing::ValuesIn(Concat(SignCases<AFloat>(),  //
                                              SignCases<f32>(),
                                              SignCases<f16>()))));

template <typename T>
std::vector<Case> StepCases() {
    return {
        C({T(0), T(0)}, T(1.0)),
        C({T(0), T(0.5)}, T(1.0)),
        C({T(0.5), T(0)}, T(0.0)),
        C({T(1), T(0.5)}, T(0.0)),
        C({T(0.5), T(1)}, T(1.0)),
        C({T(1.5), T(1)}, T(0.0)),
        C({T(1), T(1.5)}, T(1.0)),
        C({T(-1), T(1)}, T(1.0)),
        C({T(-1), T(1)}, T(1.0)),
        C({T(1), T(-1)}, T(0.0)),
        C({T(-1), T(-1.5)}, T(0.0)),
        C({T(-1.5), T(-1)}, T(1.0)),
        C({T::Highest(), T::Lowest()}, T(0.0)),
        C({T::Lowest(), T::Highest()}, T(1.0)),

        // Vector tests
        C({Vec(T(0), T(0)), Vec(T(0), T(0))}, Vec(T(1.0), T(1.0))),
        C({Vec(T(-1), T(1)), Vec(T(0), T(0))}, Vec(T(1.0), T(0.0))),
        C({Vec(T::Highest(), T::Lowest()), Vec(T::Lowest(), T::Highest())}, Vec(T(0.0), T(1.0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Step,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kStep),
                     testing::ValuesIn(Concat(StepCases<AFloat>(),  //
                                              StepCases<f32>(),
                                              StepCases<f16>()))));

std::vector<Case> QuantizeToF16Cases() {
    (void)E({Vec(0_f, 0_f)}, "");  // Currently unused, but will be soon.
    return {
        C({0_f}, 0_f),    //
        C({-0_f}, -0_f),  //
        C({1_f}, 1_f),    //
        C({-1_f}, -1_f),  //

        //   0.00006106496 quantized to 0.000061035156 = 0x1p-14
        C({0.00006106496_f}, 0.000061035156_f),    //
        C({-0.00006106496_f}, -0.000061035156_f),  //

        //   1.0004883 quantized to 1.0 = 0x1p0
        C({1.0004883_f}, 1.0_f),    //
        C({-1.0004883_f}, -1.0_f),  //

        //   8196.0 quantized to 8192.0 = 0x1p13
        C({8196_f}, 8192_f),    //
        C({-8196_f}, -8192_f),  //

        // Value in subnormal f16 range
        C({0x0.034p-14_f}, 0x0.034p-14_f),    //
        C({-0x0.034p-14_f}, -0x0.034p-14_f),  //
        C({0x0.068p-14_f}, 0x0.068p-14_f),    //
        C({-0x0.068p-14_f}, -0x0.068p-14_f),  //

        //   0x0.06b7p-14 quantized to 0x0.068p-14
        C({0x0.06b7p-14_f}, 0x0.068p-14_f),    //
        C({-0x0.06b7p-14_f}, -0x0.068p-14_f),  //

        // Value out of f16 range
        C({65504.003_f}, 65504_f),     //
        C({-65504.003_f}, -65504_f),   //
        C({0x1.234p56_f}, 65504_f),    //
        C({-0x4.321p65_f}, -65504_f),  //

        // Vector tests
        C({Vec(0_f, -0_f)}, Vec(0_f, -0_f)),  //
        C({Vec(1_f, -1_f)}, Vec(1_f, -1_f)),  //

        C({Vec(0.00006106496_f, -0.00006106496_f, 1.0004883_f, -1.0004883_f)},
          Vec(0.000061035156_f, -0.000061035156_f, 1.0_f, -1.0_f)),

        C({Vec(8196_f, 8192_f, 0x0.034p-14_f)}, Vec(8192_f, 8192_f, 0x0.034p-14_f)),

        C({Vec(0x0.034p-14_f, -0x0.034p-14_f, 0x0.068p-14_f, -0x0.068p-14_f)},
          Vec(0x0.034p-14_f, -0x0.034p-14_f, 0x0.068p-14_f, -0x0.068p-14_f)),

        C({Vec(65504.003_f, 0x1.234p56_f)}, Vec(65504_f, 65504_f)),
        C({Vec(-0x1.234p56_f, -65504.003_f)}, Vec(-65504_f, -65504_f)),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    QuantizeToF16,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kQuantizeToF16),
                     testing::ValuesIn(QuantizeToF16Cases())));

}  // namespace
}  // namespace tint::resolver
