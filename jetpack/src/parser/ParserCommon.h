//
// Created by Duzhong Chen on 2019/9/6.
//

#pragma once

#include <optional>
#include <memory>
#include <stack>
#include <functional>
#include <unordered_set>
#include "tokenizer/Token.h"
#include "ParseErrorHandler.h"
#include "utils/Common.h"
#include "macros.h"
#include "tokenizer/Scanner.h"
#include "SyntaxNodes.h"
#include "ParserContext.h"

namespace jetpack::parser {

    template <typename T>
    class NodeCreatedEventEmitter {
    public:

        using Callback = std::function<void(T*)>;

        inline void Emit(T* obj) {
            for (auto& fun : callbacks) {
                fun(obj);
            }
        }

        inline void On(Callback cb) {
            callbacks.push_back(std::move(cb));
        }

        std::vector<Callback> callbacks;

    };

    template <typename RetT, typename T>
    class NodeCreatedEventEmitterRet {
    public:

        using Callback = std::function<RetT(T*)>;

        inline RetT Emit(T* obj) {
            return callback(obj);
        }

        inline void On(Callback cb) {
            callback = std::move(cb);
        }

        Callback callback;

    };

    class ParserCommon {
    public:
        struct FormalParameterOptions {
            bool simple = true;
            NodeList<SyntaxNode> params;
            HashSet<std::string> param_set;
            std::optional<Token> stricted;
            std::optional<Token> first_restricted;
            std::string message;
        };

        ParserCommon(AstContext& ctx, std::string_view src, const Config& config);
        ParserCommon(AstContext& ctx, Sp<MemoryViewOwner> src, const Config& config);
        ParserCommon(std::shared_ptr<ParserContext> state);
        ParserCommon(const ParserCommon&) = delete;
        ParserCommon(ParserCommon&&) = delete;

        ParserCommon& operator=(const ParserCommon&) = delete;
        ParserCommon& operator=(ParserCommon&&) = delete;

        void DecorateToken(Token& );

        Token NextToken();

        Token NextRegexToken();

        inline std::string_view GetTokenRaw(const Token& token) {
            return ctx->scanner_->Source()->View().substr
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
                ctx->start_marker_.cursor,
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

        bool MatchContextualKeyword(const std::string& keyword);
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

        inline Sp<ParserContext> Context() const {
            return ctx;
        }

    private:
        ParseError UnexpectedToken(const Token& tok);
        ParseError UnexpectedToken(const Token& tok, const std::string& message);

    protected:
        template<typename T, typename ...Args>
        typename std::enable_if<std::is_base_of<SyntaxNode, T>::value, T*>::type
        Alloc(Args && ...args) {
            return ctx->ast_context_.Alloc<T, Args...>(std::forward<Args>(args)...);
        }

        template<typename T>
        typename std::enable_if<std::is_base_of<SyntaxNode, T>::value, T*>::type
        Finalize(const ParserContext::Marker& marker, T* from) {
            from->range = std::make_pair(marker.cursor.u8, LastMarker().cursor.u8);

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

        std::unique_ptr<LeftValueScope> left_scope_;

        std::shared_ptr<ParserContext> ctx;

    };

}
