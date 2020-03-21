//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <string>
#include <utility>
#include <Utils.h>

#define DEF_TOKEN(D) \
    D(BooleanLiteral) \
    D(EOF_) \
    D(Identifier) \
    D(NullLiteral) \
    D(NumericLiteral) \
    D(StringLiteral) \
    D(RegularExpression) \
    D(Template) \
    D(LeftParen) \
    D(RightParen) \
    D(LeftBracket)\
    D(RightBracket) \
    D(LeftBrace) \
    D(RightBrace) \
    D(Dot) \
    D(Spread) \
    D(Semicolon) \
    D(Comma) \
    D(Colon) \
    D(Ask) \
    D(Wave) \
    D(LessThan) \
    D(GreaterThan) \
    D(StrictEqual) \
    D(StrictNotEqual) \
    D(Equal) \
    D(NotEqual) \
    D(LessEqual) \
    D(GreaterEqual) \
    D(Mod) \
    D(Xor) \
    D(BitAnd) \
    D(BitOr) \
    D(And) \
    D(Or) \
    D(Assign) \
    D(BitAndAssign) \
    D(BitOrAssign) \
    D(BitXorAssign) \
    D(ModAssign) \
    D(PlusAssign) \
    D(MinusAssign) \
    D(MulAssign) \
    D(DivAssign) \
    D(PowAssign) \
    D(LeftShiftAssign) \
    D(RightShiftAssign) \
    D(ZeroFillRightShiftAssign) \
    D(Plus) \
    D(Minus) \
    D(Mul) \
    D(Div) \
    D(Pow) \
    D(Increase) \
    D(Decrease) \
    D(LeftShift) \
    D(RightShift) \
    D(ZeroFillRightShift) \
    D(Not) \
    D(Arrow) \
    D(K_If) \
    D(K_In) \
    D(K_Do) \
    D(K_Var) \
    D(K_For) \
    D(K_New) \
    D(K_Try) \
    D(K_Let) \
    D(K_This) \
    D(K_Else) \
    D(K_Case) \
    D(K_Void) \
    D(K_With) \
    D(K_Enum) \
    D(K_While) \
    D(K_Break) \
    D(K_Catch) \
    D(K_Throw) \
    D(K_Const) \
    D(K_Yield) \
    D(K_Class) \
    D(K_Super) \
    D(K_Return) \
    D(K_Typeof) \
    D(K_Delete) \
    D(K_Switch) \
    D(K_Export) \
    D(K_Import) \
    D(K_Default) \
    D(K_Finally) \
    D(K_Extends) \
    D(K_Function) \
    D(K_Continue) \
    D(K_Debugger) \
    D(K_Instanceof) \
    D(KS_Implements) \
    D(KS_Interface) \
    D(KS_Package) \
    D(KS_Private) \
    D(KS_Protected) \
    D(KS_Public) \
    D(KS_Static) \


#define DD(NAME) NAME,

enum class JsTokenType {
    Invalid,

    DEF_TOKEN(DD)

};

#undef DD

inline bool IsPunctuatorToken(JsTokenType t) {
    return t >= JsTokenType::LeftParen && t <= JsTokenType::Arrow;
}

inline bool IsKeywordToken(JsTokenType t) {
    return t >= JsTokenType::K_If && t <= JsTokenType::KS_Static;
}

static const char* TokenTypeToCString(JsTokenType tt);

struct Position {
    std::uint32_t line_;
    std::uint32_t column_;

    Position(): line_(0u), column_(0u) {}
    Position(std::uint32_t line, std::uint32_t column):
        line_(line), column_(column) {}
};

struct SourceLocation {
public:
    Position start_;
    Position end_;

    SourceLocation() = default;
    SourceLocation(Position start, Position end):
    start_(start), end_(end) {}
};

class Token {
public:
    JsTokenType type_ = JsTokenType::Invalid;
    UString value_;
    SourceLocation loc_;
    std::uint32_t line_number_;
    std::uint32_t line_start_;
    std::pair<std::int32_t, std::int32_t> range_;
    bool octal_ = false;
    bool head_ = false;
    bool tail_ = false;
    char16_t cooked_;

};
