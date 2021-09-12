//
// Created by Duzhong Chen on 2020/4/3.
//

#include <string>
#include <boost/lexical_cast.hpp>
#include "parser/NodesMaker.h"
#include "ConstantFolding.h"
#include "utils/string/UString.h"

namespace jetpack {

    inline Sp<Literal> MakeIntLiteral(std::int32_t tmp) {
        auto lit = std::make_shared<Literal>();
        lit->ty = Literal::Ty::Double;
        lit->str_ = std::to_string(tmp);
        lit->raw = lit->str_;
        return lit;
    }

// tricks from https://stackoverflow.com/questions/11544073/how-do-i-deal-with-the-max-macro-in-windows-h-colliding-with-max-in-std
#define DUMMY

    inline bool IsValieResult(std::int64_t tmp) {
        return tmp >= std::numeric_limits<std::int32_t>::min DUMMY () && tmp <= std::numeric_limits<std::int32_t>::max DUMMY ();
    }

    Sp<Expression> ContantFolding::TryBinaryExpression(const Sp<BinaryExpression> &binary) {
        if (binary->left->type == SyntaxNodeType::Literal
            && binary->right->type == SyntaxNodeType::Literal) {

            auto left_lit = std::dynamic_pointer_cast<Literal>(binary->left);
            auto right_lit = std::dynamic_pointer_cast<Literal>(binary->right);

            if (binary->operator_ == "+" && left_lit->ty == Literal::Ty::String && right_lit->ty == Literal::Ty::String) {
                std::string result = left_lit->str_ + right_lit->str_;
                return MakeStringLiteral(result);
            } else if (left_lit->ty == Literal::Ty::Double && right_lit->ty == Literal::Ty::Double) {
                int32_t left_int, right_int;
                try {
                    left_int = boost::lexical_cast<int32_t>(left_lit->str_);
                    right_int = boost::lexical_cast<int32_t>(right_lit->str_);
                } catch (const boost::bad_lexical_cast& ex) {
                    return binary;
                }

                int64_t tmp_result = 0;
                if (binary->operator_ == "+") {
                    tmp_result = left_int + right_int;
                } else if (binary->operator_ == "-") {
                    tmp_result = left_int - right_int;
                } else {
                    return binary;
                }

                if (!IsValieResult(tmp_result)) {
                    return binary;
                }
                return MakeIntLiteral(static_cast<std::int32_t>(tmp_result));
            }
        }
        return binary;
    }

}
