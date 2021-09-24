//
// Created by Duzhong Chen on 2019/10/30.
//

#include "TypescriptParser.h"
#include "Parser.hpp"

namespace jetpack::parser {

    TypeScriptParser::TypeScriptParser(std::shared_ptr<ParserContext> state):
    ParserCommon(std::move(state)) {
    }

    TSTypeAliasDeclaration* TypeScriptParser::ParseTypeAliasDeclaration() {
        auto start_marker = CreateStartMarker();

        if (!MatchContextualKeyword("type")) {
            ThrowUnexpectedToken(ctx->lookahead_);
        }

        NextToken();

        auto node = Alloc<TSTypeAliasDeclaration>();

        Parser parser(ctx);
        node->id = parser.ParseIdentifierName();

        if (Match(JsTokenType::LessThan)) {
            node->type_parameters = { ParseTypeParameterDeclaration() };
        }

        Expect(JsTokenType::Assign);

        node->type_annotation = ParseType();

        return Finalize(start_marker, node);
    }

    TSType* TypeScriptParser::ParseType() {
        // TODO:
        return ParseNonConditionalType();
    }

    TSType* TypeScriptParser::ParseNonConditionalType() {
        if (IsStartOfFunctionType()) {
            return ParseFunctionType();
        }
        if (Match(JsTokenType::K_New)) {
            return ParseConstructorType();
        }
        return ParseUnionTypeOrHigher();
    }

    TSFunctionType* TypeScriptParser::ParseFunctionType() {
        auto start_marker = CreateStartMarker();

        auto node = Alloc<TSFunctionType>();

        // TODO: function type

        return Finalize(start_marker, node);
    }

    TSConstructorType* TypeScriptParser::ParseConstructorType() {
        auto start_marker = CreateStartMarker();

        auto node = Alloc<TSConstructorType>();

        return Finalize(start_marker, node);
    }

    TSThisType* TypeScriptParser::ParseThisType() {
        auto start_marker = CreateStartMarker();

        Expect(JsTokenType::K_This);

        auto node = Alloc<TSThisType>();
        return Finalize(start_marker, node);
    }

    TSLiteralType* TypeScriptParser::ParseLiteralTypeNode() {
        auto start_marker = CreateStartMarker();

        auto node = Alloc<TSLiteralType>();

        return Finalize(start_marker, node);
    }

    bool TypeScriptParser::IsStartOfFunctionType() {
        return false;
    }

    TSType* TypeScriptParser::ParseUnionTypeOrHigher() {
        ThrowUnexpectedToken(ctx->lookahead_);
        return nullptr;
    }

    TSTypeParameterDeclaration* TypeScriptParser::ParseTypeParameterDeclaration() {
        ThrowUnexpectedToken(ctx->lookahead_);
        return nullptr;
    }

}
