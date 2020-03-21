//
// Created by Duzhong Chen on 2019/10/30.
//

#include "TypescriptParser.h"
#include "Parser.hpp"

namespace rocket_bundle::parser {

    TypeScriptParser::TypeScriptParser(std::shared_ptr<ParserContext> state):
    ParserCommon(std::move(state)) {
    }

    Sp<TSTypeAliasDeclaration> TypeScriptParser::ParseTypeAliasDeclaration() {
        auto start_marker = CreateStartMarker();

        if (!MatchContextualKeyword(u"type")) {
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

    Sp<TSType> TypeScriptParser::ParseType() {
        // TODO:
        return ParseNonConditionalType();
    }

    Sp<TSType> TypeScriptParser::ParseNonConditionalType() {
        if (IsStartOfFunctionType()) {
            return ParseFunctionType();
        }
        if (Match(JsTokenType::K_New)) {
            return ParseConstructorType();
        }
        return ParseUnionTypeOrHigher();
    }

    Sp<TSFunctionType> TypeScriptParser::ParseFunctionType() {
        auto start_marker = CreateStartMarker();

        auto node = Alloc<TSFunctionType>();

        // TODO: function type

        return Finalize(start_marker, node);
    }

    Sp<TSConstructorType> TypeScriptParser::ParseConstructorType() {
        auto start_marker = CreateStartMarker();

        auto node = Alloc<TSConstructorType>();

        return Finalize(start_marker, node);
    }

    Sp<TSThisType> TypeScriptParser::ParseThisType() {
        auto start_marker = CreateStartMarker();

        Expect(JsTokenType::K_This);

        auto node = Alloc<TSThisType>();
        return Finalize(start_marker, node);
    }

    Sp<TSLiteralType> TypeScriptParser::ParseLiteralTypeNode() {
        auto start_marker = CreateStartMarker();

        auto node = Alloc<TSLiteralType>();

        return Finalize(start_marker, node);
    }

    bool TypeScriptParser::IsStartOfFunctionType() {
        return false;
    }

    Sp<TSType> TypeScriptParser::ParseUnionTypeOrHigher() {
        ThrowUnexpectedToken(ctx->lookahead_);
        return nullptr;
    }

    Sp<TSTypeParameterDeclaration> TypeScriptParser::ParseTypeParameterDeclaration() {
        ThrowUnexpectedToken(ctx->lookahead_);
        return nullptr;
    }

}
