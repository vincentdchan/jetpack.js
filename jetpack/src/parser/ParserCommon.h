//
// Created by Duzhong Chen on 2019/9/6.
//

#pragma once

#include <optional>
#include <memory>
#include <stack>
#include <functional>
#include <unordered_set>
#include "../tokenizer/Token.h"
#include "ParseErrorHandler.h"
#include "Utils.h"
#include "../macros.h"
#include "../tokenizer/Scanner.h"
#include "SyntaxNodes.h"
#include "ParserContext.h"

namespace jetpack::parser {

    template <typename T>
    class NodeCreatedEventEmitter {
    public:

        using Callback = std::function<void(const Sp<T>&)>;

        inline void Emit(const Sp<T>& obj) {
            for (auto& fun : callbacks) {
                fun(obj);
            }
        }

        inline void On(Callback cb) {
            callbacks.push_back(std::move(cb));
        }

        std::vector<Callback> callbacks;

    };

    class ParserCommon {
    public:
        struct FormalParameterOptions {
            bool simple = true;
            std::vector<Sp<SyntaxNode>> params;
            std::unordered_set<UString> param_set;
            std::optional<Token> stricted;
            std::optional<Token> first_restricted;
            std::string message;
        };

        ParserCommon(std::shared_ptr<ParserContext> state);
        ParserCommon(const ParserCommon&) = delete;
        ParserCommon(ParserCommon&&) = delete;

        ParserCommon& operator=(const ParserCommon&) = delete;
        ParserCommon& operator=(ParserCommon&&) = delete;

        void DecorateToken(Token& );

        Token NextToken();

        Token NextRegexToken();

        inline UString GetTokenRaw(const Token& token) {
            return ctx->scanner_->Source().substr
            (token.range.first, token.range.second - token.range.first);
        }

        void TolerateError(const std::string& message);

        void ThrowUnexpectedToken(const Token& tok);
        void ThrowUnexpectedToken(const Token& tok, const std::string& message);
        void TolerateUnexpectedToken(const Token& tok);
        void TolerateUnexpectedToken(const Token& tok, const std::string& message);

        void ThrowError(const std::string& message);
        void ThrowError(const std::string& message, const std::string& arg);

        ParserContext::Marker StartNode(Token& tok, uint32_t last_line_start = 0);

        inline Token& Lookahead() {
            return ctx->lookahead_;
        }

        inline ParserContext::Marker CreateStartMarker() {
            return {
                ctx->start_marker_.index,
                ctx->start_marker_.line,
                ctx->start_marker_.column
            };
        }

        void ExpectCommaSeparator();

        inline void Expect(JsTokenType t) {
            Token token = NextToken();

            if (token.type != t) {
                ThrowUnexpectedToken(token);
            }
        }

        [[nodiscard]] inline bool Match(JsTokenType t) const {
            return ctx->lookahead_.type == t;
        }

        bool MatchContextualKeyword(const UString& keyword);
        bool MatchAssign();

        void ConsumeSemicolon();

        void CollectComments();

        [[nodiscard]] int BinaryPrecedence(const Token& token) const;

        static bool IsIdentifierName(Token&);

        inline void SetLastMarker(const ParserContext::Marker& marker) {
            ctx->last_marker_ = marker;
        }

        [[nodiscard]] inline ParserContext::Marker LastMarker() const {
            return ctx->last_marker_;
        }

        inline void SetStartMarker(const ParserContext::Marker& marker) {
            ctx->start_marker_ = marker;
        }

        [[nodiscard]] inline ParserContext::Marker StartMarker() const {
            return ctx->start_marker_;
        }

        inline void Assert(bool value, std::string message) {
            if (!value) {
                throw ParseAssertFailed(
                    std::move(message), ctx->last_marker_.line,
                    ctx->last_marker_.column
                );
            }
        }

    private:
        ParseError UnexpectedToken(const Token& tok);
        ParseError UnexpectedToken(const Token& tok, const std::string& message);

    protected:
        template<typename T, typename ...Args>
        typename std::enable_if<std::is_base_of<SyntaxNode, T>::value, Sp<T>>::type
        Alloc(Args && ...args) {
            T* ptr = new T(std::forward<Args>(args)...);
            return Sp<T>(ptr);
        }

        template<typename T>
        typename std::enable_if<std::is_base_of<SyntaxNode, T>::value, Sp<T>>::type
        Finalize(const ParserContext::Marker& marker, const Sp<T>& from) {
            from->range = std::make_pair(marker.index, LastMarker().index);

            from->location.fileId = ctx->fileIndex;

            from->location.start = Position {
                marker.line,
                marker.column,
            };

            from->location.end = Position {
                LastMarker().line,
                LastMarker().column,
            };

            return from;
        }

        std::shared_ptr<ParserContext> ctx;

    };

}
