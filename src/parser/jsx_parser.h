//
// Created by Duzhong Chen on 2019/10/25.
//

#pragma once

#include "parser.hpp"
#include <vector>
#include <robin_hood.h>

namespace parser {

    class JSXParser {
    public:
        struct MetaJSXElement {
        public:
            MetaJSXElement() = default;

            ParserCommon::Marker start_marker_;
            Sp<JSXOpeningElement> opening_;
            std::optional<Sp<JSXClosingElement>> closing_;
            std::vector<Sp<SyntaxNode>> children_;

            void* operator new(std::size_t size);
            void operator delete(void* ptr);

        };

        static UString GetQualifiedElementName(const Sp<SyntaxNode>& node);

        JSXParser(Parser* parser);
        JSXParser(const JSXParser& parser) = delete;
        JSXParser(JSXParser&&) = delete;

        JSXParser& operator=(const JSXParser& parser) = delete;
        JSXParser& operator=(JSXParser&&) = delete;

        Sp<JSXElement> ParseJSXRoot();

        Sp<JSXElement> ParseJSXElement();

        Sp<JSXOpeningElement> ParseJSXOpeningElement();

        Sp<SyntaxNode> ParseJSXElementName();

        std::vector<Sp<SyntaxNode>> ParseJSXAttributes();

        Sp<JSXIdentifier> ParseJSXIdentifier();

        Sp<MetaJSXElement> ParseComplexJSXElement(Sp<MetaJSXElement> el);

        Sp<SyntaxNode> ParseJSXBoundaryElement();

        std::vector<Sp<SyntaxNode>> ParseJSXChildren();

        Sp<JSXExpressionContainer> ParseJSXExpressionContainer();

        Sp<JSXSpreadAttribute> ParseJSXSpreadAttribute();

        Sp<JSXAttribute> ParseJSXNameValueAttribute();

        Sp<SyntaxNode> ParseJSXAttributeName();

        Sp<SyntaxNode> ParseJSXAttributeValue();

        Sp<JSXExpressionContainer> ParseJSXExpressionAttribute();

        Sp<Literal> ParseJSXStringLiteralAttribute();

        void StartJSX();
        void FinishJSX();
        void ReEnterJSX();

    private:
        Parser& parser_;

        Token LexJSX();
        Token NextJSXToken();
        Token NextJSXText();
        Token PeekJSXToken();

        UString ScanXHTMLEntity(char16_t quote);

        static std::unique_ptr<robin_hood::unordered_map<std::u16string, char16_t>> XHTMLEntities;

        void InitXHTMLEntities();

    public:
        inline bool Match(JsTokenType jt) {
            auto next = PeekJSXToken();
            return next.type_ == jt;
        }

        inline void Expect(JsTokenType t) {
            Token token = NextJSXToken();
            if (!IsPunctuatorToken(token.type_)) {
                parser_.ThrowUnexpectedToken(token);
            }
        }

    };

}
