//
// Created by Duzhong Chen on 2019/10/25.
//

#pragma once

#include "Parser.hpp"
#include "utils/Common.h"
#include <vector>

namespace jetpack::parser {

    class JSXParser: private ParserCommon {
    public:
        struct MetaJSXElement {
        public:
            MetaJSXElement() = default;

            ParserContext::Marker start_marker_;
            JSXOpeningElement* opening_;
            std::optional<JSXClosingElement*> closing_;
            std::vector<SyntaxNode*> children_;

        };

        static std::string GetQualifiedElementName(SyntaxNode* node);

        JSXParser(Parser& parent, std::shared_ptr<ParserContext> ctx);
        JSXParser(const JSXParser& parser) = delete;
        JSXParser(JSXParser&&) = delete;

        JSXParser& operator=(const JSXParser& parser) = delete;
        JSXParser& operator=(JSXParser&&) = delete;

        Expression* ParseJSXRoot(Scope& scope);

        Expression* TranspileJSX(Scope& scope, JSXElement* jsx);

        std::vector<SyntaxNode*>
        TranspileJSXChildren(Scope& scope, const std::vector<SyntaxNode*>& children);

        JSXElement* ParseJSXElement(Scope& scope);

        JSXOpeningElement* ParseJSXOpeningElement(Scope& scope);

        SyntaxNode* ParseJSXElementName(Scope& scope);

        std::vector<SyntaxNode*> ParseJSXAttributes(Scope& scope);

        JSXIdentifier* ParseJSXIdentifier();

        Sp<MetaJSXElement> ParseComplexJSXElement(Scope& scope, Sp<MetaJSXElement> el);

        SyntaxNode* ParseJSXBoundaryElement(Scope& scope);

        std::vector<SyntaxNode*> ParseJSXChildren(Scope& scope);

        JSXExpressionContainer* ParseJSXExpressionContainer(Scope& scope);

        JSXSpreadAttribute* ParseJSXSpreadAttribute(Scope& scope);

        JSXAttribute* ParseJSXNameValueAttribute(Scope& scope);

        SyntaxNode* ParseJSXAttributeName();

        SyntaxNode* ParseJSXAttributeValue(Scope& scope);

        JSXExpressionContainer* ParseJSXExpressionAttribute(Scope& scope);

        Literal* ParseJSXStringLiteralAttribute();

        void StartJSX();
        void FinishJSX();
        void ReEnterJSX();

    private:
        Token LexJSX();
        Token NextJSXToken();
        Token NextJSXText();
        Token PeekJSXToken();

        std::string ScanXHTMLEntity(char quote);

        static std::unique_ptr<HashMap<std::string, char16_t>> XHTMLEntities;

        void InitXHTMLEntities();

        Parser& parent_;

    public:
        inline bool JSXMatch(JsTokenType jt) {
            auto next = PeekJSXToken();
            return next.type == jt;
        }

        inline void JSXExpect(JsTokenType t) {
            Token token = NextJSXToken();
            if (!IsPunctuatorToken(token.type)) {
                ThrowUnexpectedToken(token);
            }
        }

    };

}
