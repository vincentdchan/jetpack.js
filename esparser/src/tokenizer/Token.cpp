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

            case JsTokenType::K_If:
                return UStr("if");

            case JsTokenType::K_In:
                return UStr("in");

            case JsTokenType::K_Do:
                return UStr("do");

            case JsTokenType::K_Var:
                return UStr("var");

            case JsTokenType::K_For:
                return UStr("for");

            case JsTokenType::K_New:
                return UStr("new");

            case JsTokenType::K_Try:
                return UStr("try");

            case JsTokenType::K_Let:
                return UStr("let");

            case JsTokenType::K_This:
                return UStr("this");

            case JsTokenType::K_Else:
                return UStr("else");

            case JsTokenType::K_Case:
                return UStr("case");

            case JsTokenType::K_Void:
                return UStr("void");

            case JsTokenType::K_With:
                return UStr("with");

            case JsTokenType::K_Enum:
                return UStr("enum");

            case JsTokenType::K_While:
                return UStr("while");

            case JsTokenType::K_Break:
                return UStr("break");

            case JsTokenType::K_Catch:
                return UStr("catch");

            case JsTokenType::K_Throw:
                return UStr("throw");

            case JsTokenType::K_Const:
                return UStr("const");

            case JsTokenType::K_Yield:
                return UStr("yield");

            case JsTokenType::K_Class:
                return UStr("class");

            case JsTokenType::K_Super:
                return UStr("super");

            case JsTokenType::K_Return:
                return UStr("return");

            case JsTokenType::K_Typeof:
                return UStr("typeof");

            case JsTokenType::K_Delete:
                return UStr("delete");

            case JsTokenType::K_Switch:
                return UStr("switch");

            case JsTokenType::K_Export:
                return UStr("export");

            case JsTokenType::K_Import:
                return UStr("import");

            case JsTokenType::K_Default:
                return UStr("default");

            case JsTokenType::K_Finally:
                return UStr("finally");

            case JsTokenType::K_Extends:
                return UStr("extends");

            case JsTokenType::K_Function:
                return UStr("function");

            case JsTokenType::K_Continue:
                return UStr("continue");

            case JsTokenType::K_Debugger:
                return UStr("debugger");

            case JsTokenType::K_Instanceof:
                return UStr("instanceof");

            default:
                J_ASSERT(false);
                return UStr("<internal error>");

        }

    }

}
