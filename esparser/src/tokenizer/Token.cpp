//
// Created by Duzhong Chen on 2019/9/3.
//

#include "Token.h"

namespace jetpack {

#define DD(NAME) \
    case JsTokenType::NAME: \
        return #NAME;

    const char *TokenTypeToCString(JsTokenType tt) {
        switch (tt) {

            DEF_TOKEN(DD)

            default:
                return "<invalid>";
        }
    }

#undef DD

    UStringView TokenTypeToLiteral(JsTokenType tt) {
        switch (tt) {
            case JsTokenType::TrueLiteral:
                return UStr("true");

            case JsTokenType::FalseLiteral:
                return UStr("false");

            case JsTokenType::NullLiteral:
                return UStr("null");

            case JsTokenType::LeftParen:
                return UStr("(");

            case JsTokenType::RightParen:
                return UStr(")");

            case JsTokenType::LeftBracket:
                return UStr("{");

            case JsTokenType::RightBracket:
                return UStr("}");

            case JsTokenType::LeftBrace:
                return UStr("[");

            case JsTokenType::RightBrace:
                return UStr("]");

            case JsTokenType::Dot:
                return UStr(".");

            case JsTokenType::Spread:
                return UStr("...");

            case JsTokenType::Semicolon:
                return UStr(";");

            case JsTokenType::Comma:
                return UStr(",");

            case JsTokenType::Colon:
                return UStr(":");

            case JsTokenType::Ask:
                return UStr("?");

            case JsTokenType::Wave:
                return UStr("~");

            case JsTokenType::LessThan:
                return UStr("<");

            case JsTokenType::GreaterThan:
                return UStr(">");

            case JsTokenType::StrictEqual:
                return UStr("===");

            case JsTokenType::StrictNotEqual:
                return UStr("!==");

            case JsTokenType::Equal:
                return UStr("==");

            case JsTokenType::NotEqual:
                return UStr("!=");

            case JsTokenType::LessEqual:
                return UStr("<=");

            case JsTokenType::GreaterEqual:
                return UStr(">=");

            case JsTokenType::Mod:
                return UStr("%");

            case JsTokenType::Xor:
                return UStr("^");

            case JsTokenType::BitAnd:
                return UStr("&");

            case JsTokenType::BitOr:
                return UStr("|");

            case JsTokenType::And:
                return UStr("&&");

            case JsTokenType::Or:
                return UStr("||");

            case JsTokenType::Assign:
                return UStr("=");

            case JsTokenType::BitAndAssign:
                return UStr("&=");

            case JsTokenType::BitOrAssign:
                return UStr("|=");

            case JsTokenType::BitXorAssign:
                return UStr("^=");

            case JsTokenType::ModAssign:
                return UStr("%=");

            case JsTokenType::PlusAssign:
                return UStr("+=");

            case JsTokenType::MinusAssign:
                return UStr("-=");

            case JsTokenType::MulAssign:
                return UStr("*=");

            case JsTokenType::DivAssign:
                return UStr("/=");

            case JsTokenType::PowAssign:
                return UStr("**=");

            case JsTokenType::LeftShiftAssign:
                return UStr("<<=");

            case JsTokenType::RightShiftAssign:
                return UStr(">>=");

            case JsTokenType::ZeroFillRightShiftAssign:
                return UStr(">>>=");

            case JsTokenType::Plus:
                return UStr("+");

            case JsTokenType::Minus:
                return UStr("-");

            case JsTokenType::Mul:
                return UStr("*");

            case JsTokenType::Div:
                return UStr("/");

            case JsTokenType::Pow:
                return UStr("**");

            case JsTokenType::Increase:
                return UStr("++");

            case JsTokenType::Decrease:
                return UStr("--");

            case JsTokenType::LeftShift:
                return UStr("<<");

            case JsTokenType::RightShift:
                return UStr(">>");

            case JsTokenType::ZeroFillRightShift:
                return UStr(">>>");

            case JsTokenType::Not:
                return UStr("!");

            case JsTokenType::Arrow:
                return UStr("=>");

            default:
                J_ASSERT(false);
                return UStr("");

        }

    }

}
