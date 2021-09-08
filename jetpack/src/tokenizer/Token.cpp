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

    std::string_view TokenTypeToLiteral(JsTokenType tt) {
        switch (tt) {
            case JsTokenType::TrueLiteral:
                return "true";

            case JsTokenType::FalseLiteral:
                return "false";

            case JsTokenType::NullLiteral:
                return "null";

            case JsTokenType::LeftParen:
                return "(";

            case JsTokenType::RightParen:
                return ")";

            case JsTokenType::LeftBracket:
                return "{";

            case JsTokenType::RightBracket:
                return "}";

            case JsTokenType::LeftBrace:
                return "[";

            case JsTokenType::RightBrace:
                return "]";

            case JsTokenType::Dot:
                return ".";

            case JsTokenType::Spread:
                return "...";

            case JsTokenType::Semicolon:
                return ";";

            case JsTokenType::Comma:
                return ",";

            case JsTokenType::Colon:
                return ":";

            case JsTokenType::Ask:
                return "?";

            case JsTokenType::Wave:
                return "~";

            case JsTokenType::LessThan:
                return "<";

            case JsTokenType::GreaterThan:
                return ">";

            case JsTokenType::StrictEqual:
                return "===";

            case JsTokenType::StrictNotEqual:
                return "!==";

            case JsTokenType::Equal:
                return "==";

            case JsTokenType::NotEqual:
                return "!=";

            case JsTokenType::LessEqual:
                return "<=";

            case JsTokenType::GreaterEqual:
                return ">=";

            case JsTokenType::Mod:
                return "%";

            case JsTokenType::Xor:
                return "^";

            case JsTokenType::BitAnd:
                return "&";

            case JsTokenType::BitOr:
                return "|";

            case JsTokenType::And:
                return "&&";

            case JsTokenType::Or:
                return "||";

            case JsTokenType::Assign:
                return "=";

            case JsTokenType::BitAndAssign:
                return "&=";

            case JsTokenType::BitOrAssign:
                return "|=";

            case JsTokenType::BitXorAssign:
                return "^=";

            case JsTokenType::ModAssign:
                return "%=";

            case JsTokenType::PlusAssign:
                return "+=";

            case JsTokenType::MinusAssign:
                return "-=";

            case JsTokenType::MulAssign:
                return "*=";

            case JsTokenType::DivAssign:
                return "/=";

            case JsTokenType::PowAssign:
                return "**=";

            case JsTokenType::LeftShiftAssign:
                return "<<=";

            case JsTokenType::RightShiftAssign:
                return ">>=";

            case JsTokenType::ZeroFillRightShiftAssign:
                return ">>>=";

            case JsTokenType::Plus:
                return "+";

            case JsTokenType::Minus:
                return "-";

            case JsTokenType::Mul:
                return "*";

            case JsTokenType::Div:
                return "/";

            case JsTokenType::Pow:
                return "**";

            case JsTokenType::Increase:
                return "++";

            case JsTokenType::Decrease:
                return "--";

            case JsTokenType::LeftShift:
                return "<<";

            case JsTokenType::RightShift:
                return ">>";

            case JsTokenType::ZeroFillRightShift:
                return ">>>";

            case JsTokenType::Not:
                return "!";

            case JsTokenType::Arrow:
                return "=>";

            case JsTokenType::K_If:
                return "if";

            case JsTokenType::K_In:
                return "in";

            case JsTokenType::K_Do:
                return "do";

            case JsTokenType::K_Var:
                return "var";

            case JsTokenType::K_For:
                return "for";

            case JsTokenType::K_New:
                return "new";

            case JsTokenType::K_Try:
                return "try";

            case JsTokenType::K_Let:
                return "let";

            case JsTokenType::K_This:
                return "this";

            case JsTokenType::K_Else:
                return "else";

            case JsTokenType::K_Case:
                return "case";

            case JsTokenType::K_Void:
                return "void";

            case JsTokenType::K_With:
                return "with";

            case JsTokenType::K_Enum:
                return "enum";

            case JsTokenType::K_While:
                return "while";

            case JsTokenType::K_Break:
                return "break";

            case JsTokenType::K_Catch:
                return "catch";

            case JsTokenType::K_Throw:
                return "throw";

            case JsTokenType::K_Const:
                return "const";

            case JsTokenType::K_Yield:
                return "yield";

            case JsTokenType::K_Class:
                return "class";

            case JsTokenType::K_Super:
                return "super";

            case JsTokenType::K_Return:
                return "return";

            case JsTokenType::K_Typeof:
                return "typeof";

            case JsTokenType::K_Delete:
                return "delete";

            case JsTokenType::K_Switch:
                return "switch";

            case JsTokenType::K_Export:
                return "export";

            case JsTokenType::K_Import:
                return "import";

            case JsTokenType::K_Default:
                return "default";

            case JsTokenType::K_Finally:
                return "finally";

            case JsTokenType::K_Extends:
                return "extends";

            case JsTokenType::K_Function:
                return "function";

            case JsTokenType::K_Continue:
                return "continue";

            case JsTokenType::K_Debugger:
                return "debugger";

            case JsTokenType::K_Instanceof:
                return "instanceof";

            default:
                J_ASSERT(false);
                return "<internal error>";

        }

    }

}
