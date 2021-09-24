//
// Created by Duzhong Chen on 2019/10/30.
//
#pragma once

#include <memory>
#include "ParserCommon.h"

namespace jetpack::parser {

    class Parser;

    class TypeScriptParser: private ParserCommon {
    public:
        TypeScriptParser(Parser& parser, std::shared_ptr<ParserContext> state);

        TypeScriptParser(const TypeScriptParser& tsParser) = delete;
        TypeScriptParser(TypeScriptParser&& tsParser) = delete;

        TypeScriptParser& operator=(const TypeScriptParser& tsParser) = delete;
        TypeScriptParser& operator=(TypeScriptParser&&) = delete;

        TSTypeAliasDeclaration* ParseTypeAliasDeclaration();

        TSTypeParameterDeclaration* ParseTypeParameterDeclaration();

        TSType* ParseType();

        TSType* ParseNonConditionalType();

        TSThisType* ParseThisType();

        TSFunctionType* ParseFunctionType();

        TSConstructorType* ParseConstructorType();

        TSType* ParseUnionTypeOrHigher();

        TSLiteralType* ParseLiteralTypeNode();

    private:
        Parser& parser_;
        bool IsStartOfFunctionType();

    };

}
