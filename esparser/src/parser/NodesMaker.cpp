//
// Created by Duzhong Chen on 2021/5/24.
//

#include "NodesMaker.h"

namespace jetpack {

    Sp<Identifier> MakeId(const UString& content) {
        auto id = std::make_shared<Identifier>();
        id->location.fileId = -2;
        id->name = content;
        return id;
    }

    Sp<Identifier> MakeId(const std::string& content) {
        return MakeId(UString::fromUtf8(content.c_str(), content.size()));
    }

    Sp<Literal> MakeStringLiteral(const UString& str) {
        auto lit = std::make_shared<Literal>();
        lit->ty = Literal::Ty::String;
        lit->str_ = str;
        lit->raw = u"\"" + str + u"\"";
        return lit;
    }

    /**
     * Ref: https://github.com/evanw/esbuild/blob/master/docs/architecture.md
     *
     * var require_foo = __commonJS((exports) => {
     *   exports.fn = () => 123;
     * });
     */
    void WrapModuleWithCommonJsTemplate(const Sp<Module>& module, const UString& var_name, const UString& cjs_call) {
        auto lambda = std::make_shared<ArrowFunctionExpression>();
        auto block = std::make_shared<BlockStatement>();
        block->body = std::move(module->body);
        lambda->body = std::move(block);
        for (const auto& var : module->scope->CastToModule()->own_variables) {
            if (var.second->predefined) {
                lambda->params.push_back(var.second->identifiers[0]);
            }
        }

        auto call_expr = std::make_shared<CallExpression>();
        call_expr->callee = MakeId(cjs_call);
        call_expr->arguments.push_back(std::move(lambda));

        auto def = std::make_shared<VariableDeclaration>();
        auto decl = std::make_shared<VariableDeclarator>();
        decl->id = MakeId(var_name);
        decl->init = { call_expr };
        def->kind = VarKind::Let;
        def->declarations.push_back(std::move(decl));

        module->body.push_back(def);
    }

}
