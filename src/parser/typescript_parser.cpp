//
// Created by Duzhong Chen on 2019/10/30.
//

#include "typescript_parser.h"

namespace parser {

    TypeScriptParser::TypeScriptParser(parser::Parser *parser): parser_(*parser) {
    }

    Sp<TSTypeAliasDeclaration> TypeScriptParser::ParseTypeAliasDeclaration() {
        auto start_marker = parser_.CreateStartMarker();

        if (!parser_.MatchContextualKeyword(u"type")) {
            parser_.ThrowUnexpectedToken(parser_.lookahead_);
        }

        parser_.NextToken();

        auto node = parser_.Alloc<TSTypeAliasDeclaration>();

        node->id = parser_.ParseIdentifierName();

        if (parser_.Match(JsTokenType::LessThan)) {
            node->type_parameters = { ParseTypeParameterDeclaration() };
        }

        parser_.Expect(JsTokenType::Assign);

        node->type_annotation = ParseType();

        return parser_.Finalize(start_marker, node);
    }

    Sp<TSType> TypeScriptParser::ParseType() {
        // TODO:
        return ParseNonConditionalType();
    }

    Sp<TSType> TypeScriptParser::ParseNonConditionalType() {
        if (IsStartOfFunctionType()) {
            return ParseFunctionType();
        }
        if (parser_.Match(JsTokenType::K_New)) {
            return ParseConstructorType();
        }
        return ParseUnionTypeOrHigher();
    }

    Sp<TSFunctionType> TypeScriptParser::ParseFunctionType() {
        auto start_marker = parser_.CreateStartMarker();

        auto node = parser_.Alloc<TSFunctionType>();

        // TODO: function type

        return parser_.Finalize(start_marker, node);
    }

    Sp<TSConstructorType> TypeScriptParser::ParseConstructorType() {
        auto start_marker = parser_.CreateStartMarker();

        auto node = parser_.Alloc<TSConstructorType>();

        return parser_.Finalize(start_marker, node);
    }

    Sp<TSThisType> TypeScriptParser::ParseThisType() {
        auto start_marker = parser_.CreateStartMarker();

        parser_.Expect(JsTokenType::K_This);

        auto node = parser_.Alloc<TSThisType>();
        return parser_.Finalize(start_marker, node);
    }

    Sp<TSLiteralType> TypeScriptParser::ParseLiteralTypeNode() {
        auto start_marker = parser_.CreateStartMarker();

        auto node = parser_.Alloc<TSLiteralType>();

        return parser_.Finalize(start_marker, node);
    }

}
