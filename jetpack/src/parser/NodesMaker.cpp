//
// Created by Duzhong Chen on 2021/5/24.
//

#include "NodesMaker.h"

namespace jetpack {

    Literal* MakeNull(AstContext& ctx) {
        auto null_lit = ctx.Alloc<Literal>();
        null_lit->ty = Literal::Ty::Null;
        null_lit->str_ = "null";
        null_lit->raw = "null";

        return null_lit;
    }

    Identifier* MakeId(AstContext& ctx, const SourceLocation& loc, const std::string& content) {
        auto id = ctx.Alloc<Identifier>();
        id->name = content;
        id->location = loc;
        return id;
    }

    Identifier* MakeId(AstContext& ctx, const std::string& content) {
        auto id = ctx.Alloc<Identifier>();
        id->location.fileId = -2;
        id->name = content;
        return id;
    }

    Literal* MakeStringLiteral(AstContext& ctx, const std::string& str) {
        auto lit = ctx.Alloc<Literal>();
        lit->ty = Literal::Ty::String;
        lit->str_ = str;
        lit->raw = "\"" + str + "\"";
        return lit;
    }

    /**
     * Ref: https://github.com/evanw/esbuild/blob/master/docs/architecture.md
     *
     * var require_foo = __commonJS((exports) => {
     *   exports.fn = () => 123;
     * });
     */
    void WrapModuleWithCommonJsTemplate(AstContext& ctx, Module& module, const std::string& var_name, const std::string& cjs_call) {
        auto lambda = ctx.Alloc<ArrowFunctionExpression>(std::make_unique<Scope>(ctx));
        auto block = ctx.Alloc<BlockStatement>();
        // TODO: maybe copy the nodes with context
        block->body = module.body;
        lambda->body = block;
        for (const auto& var : module.scope->CastToModule()->own_variables) {
            if (var.second->predefined) {
                lambda->params.push_back(var.second->identifiers[0]);
            }
        }

        auto call_expr = ctx.Alloc<CallExpression>();
        call_expr->callee = MakeId(ctx, cjs_call);
        call_expr->arguments.push_back(lambda);

        auto def = ctx.Alloc<VariableDeclaration>();
        auto decl = ctx.Alloc<VariableDeclarator>(std::make_unique<Scope>(ctx));
        decl->id = MakeId(ctx, var_name);
        decl->init = { call_expr };
        def->kind = VarKind::Let;
        def->declarations.push_back(decl);

        module.body.clear();
        module.body.push_back(def);
    }

}
