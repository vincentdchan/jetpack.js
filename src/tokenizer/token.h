//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <string>
#include <utility>
#include "../utils.h"

#define DEF_TOKEN(D) \
    D(BooleanLiteral) \
    D(EOF_) \
    D(Identifier) \
    D(Keyword) \
    D(NullLiteral) \
    D(NumericLiteral) \
    D(Punctuator) \
    D(StringLiteral) \
    D(RegularExpression) \
    D(Template)


#define DD(NAME) NAME,

enum class JsTokenType {
    Invalid,

    DEF_TOKEN(DD)

};

#undef DD

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
