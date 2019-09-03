//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <string>
#include <utility>

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
    std::int32_t line_;
    std::int32_t column_;

    Position(): line_(-1), column_(-1) {}
    Position(std::int32_t line, std::int32_t column):
    line_(line), column_(column) {}
};

struct Location {
public:
    Position start_;
    Position end_;

    Location(Position start, Position end):
    start_(start), end_(end) {}
};

class Token {
public:
    JsTokenType type_ = JsTokenType::Invalid;
    std::string value_;
    Location loc_;
    std::pair<std::int32_t, std::int32_t> range_;

};
