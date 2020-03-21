//
// Created by Duzhong Chen on 2019/10/25.
//

#pragma once

#include "Parser.hpp"
#include <vector>
#include <robin_hood.h>

namespace rocket_bundle::parser {

    class JSXParser: private ParserCommon {
    public:
        struct MetaJSXElement {
        public:
            MetaJSXElement() = default;

            ParserContext::Marker start_marker_;
            Sp<JSXOpeningElement> opening_;
            std::optional<Sp<JSXClosingElement>> closing_;
            std::vector<Sp<SyntaxNode>> children_;

        };

        static UString GetQualifiedElementName(const Sp<SyntaxNode>& node);

        JSXParser(std::shared_ptr<ParserContext> ctx);
        JSXParser(const JSXParser& parser) = delete;
        JSXParser(JSXParser&&) = delete;

        JSXParser& operator=(const JSXParser& parser) = delete;
        JSXParser& operator=(JSXParser&&) = delete;

        Sp<JSXElement> ParseJSXRoot(Scope& scope);

        Sp<JSXElement> ParseJSXElement(Scope& scope);

        Sp<JSXOpeningElement> ParseJSXOpeningElement(Scope& scope);

        Sp<SyntaxNode> ParseJSXElementName(Scope& scope);

        std::vector<Sp<SyntaxNode>> ParseJSXAttributes(Scope& scope);

        Sp<JSXIdentifier> ParseJSXIdentifier();

        Sp<MetaJSXElement> ParseComplexJSXElement(Scope& scope, Sp<MetaJSXElement> el);

        Sp<SyntaxNode> ParseJSXBoundaryElement(Scope& scope);

        std::vector<Sp<SyntaxNode>> ParseJSXChildren(Scope& scope);

        Sp<JSXExpressionContainer> ParseJSXExpressionContainer(Scope& scope);

        Sp<JSXSpreadAttribute> ParseJSXSpreadAttribute(Scope& scope);

        Sp<JSXAttribute> ParseJSXNameValueAttribute(Scope& scope);

        Sp<SyntaxNode> ParseJSXAttributeName();

        Sp<SyntaxNode> ParseJSXAttributeValue(Scope& scope);

        Sp<JSXExpressionContainer> ParseJSXExpressionAttribute(Scope& scope);

        Sp<Literal> ParseJSXStringLiteralAttribute();

        void StartJSX();
        void FinishJSX();
        void ReEnterJSX();

    private:
        Token LexJSX();
        Token NextJSXToken();
        Token NextJSXText();
        Token PeekJSXToken();

        UString ScanXHTMLEntity(char16_t quote);

        static std::unique_ptr<robin_hood::unordered_map<std::u16string, char16_t>> XHTMLEntities;

        void InitXHTMLEntities();

    public:
        inline bool JSXMatch(JsTokenType jt) {
            auto next = PeekJSXToken();
            return next.type_ == jt;
        }

        inline void JSXExpect(JsTokenType t) {
            Token token = NextJSXToken();
            if (!IsPunctuatorToken(token.type_)) {
                ThrowUnexpectedToken(token);
            }
        }

    };

}
