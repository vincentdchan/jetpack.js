//
// Created by Duzhong Chen on 2021/9/10.
//

#include <emscripten.h>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "ModuleResolver.h"
#include "parser/ParserCommon.h"
#include "parser/Parser.hpp"
#include "dumper/AstToJson.h"
#include "UniqueNameGenerator.h"

#ifdef __EMSCRIPTEN__

using namespace jetpack;
using namespace jetpack::parser;

static Sp<MinifyNameGenerator> RenameInnerScopes(Scope &scope, UnresolvedNameCollector* idLogger) {
    std::vector<Sp<MinifyNameGenerator>> temp;
    temp.reserve(scope.children.size());

    for (auto child : scope.children) {
        temp.push_back(RenameInnerScopes(*child, idLogger));
    }

    std::vector<std::tuple<std::string, std::string>> renames;
    auto renamer = MinifyNameGenerator::Merge(temp);

    for (auto& variable : scope.own_variables) {
        auto new_opt = renamer->Next(variable.first);
        if (new_opt.has_value()) {
            renames.emplace_back(variable.first, *new_opt);
        }
    }

    scope.BatchRenameSymbols(renames);

    return renamer;
}

inline std::string ParseAndCodeGen(std::string&& content, const ParserContext::Config& config, const CodeGen::Config& code_gen_config) {
    auto ctx = std::make_shared<ParserContext>(-1, std::move(content), config);
    Parser parser(ctx);

    auto mod = parser.ParseModule();
    mod->scope->ResolveAllSymbols(nullptr);

    if (code_gen_config.minify) {
        std::vector<Scope::PVar> variables;
        for (auto& tuple : mod->scope->own_variables) {
            variables.push_back(tuple.second);
        }

        std::sort(std::begin(variables), std::end(variables), [] (const Scope::PVar& p1, const Scope::PVar& p2) {
            return p1->identifiers.size() > p2->identifiers.size();
        });

        std::vector<Sp<MinifyNameGenerator>> result;
        for (auto child : mod->scope->children) {
            result.push_back(RenameInnerScopes(*child, nullptr));
        }

        auto name_generator = MinifyNameGenerator::Merge(result);

        // RenameSymbol() will change iterator, call it later
        std::vector<std::tuple<std::string, std::string>> rename_vec;

        // Distribute new name to root level variables
        for (auto& var : variables) {
            auto new_name_opt = name_generator->Next(var->name);

            if (new_name_opt.has_value()) {
                rename_vec.emplace_back(var->name, *new_name_opt);
            }
        }

        mod->scope->BatchRenameSymbols(rename_vec);
    }

    CodeGen codegen(code_gen_config, nullptr);
    codegen.Traverse(mod);
    return codegen.GetResult().content;
}

struct JpResult {
    uint32_t flags;
    char     content[];
};

#define JSX_FLAG 0x1
#define CONSTANT_FOLDING_FLAG 0x2
#define MINIFY_FLAG 0x100

extern "C" {

EMSCRIPTEN_KEEPALIVE
JpResult *parse_and_codegen(const char *str, uint32_t flags) {
    std::string result;
    JpResult* jp_result = nullptr;
    try {
        std::string content(str);
        ParserContext::Config parser_config = ParserContext::Config::Default();
        CodeGen::Config code_gen_config;
        parser_config.jsx = !!(flags & JSX_FLAG);
        parser_config.constant_folding = !!(flags & CONSTANT_FOLDING_FLAG);
        code_gen_config.minify = !!(flags & MINIFY_FLAG);
        result = ParseAndCodeGen(std::move(content), parser_config, code_gen_config);
    } catch (jetpack::parser::ParseError& err) {
        std::string errMsg = err.ErrorMessage();
        jp_result = reinterpret_cast<JpResult *>(::malloc(sizeof(JpResult) + errMsg.size() + 1));
        jp_result->flags = 1;
        std::memcpy(jp_result->content, errMsg.c_str(), errMsg.size());
        jp_result->content[errMsg.size()] = 0;
        return jp_result;
    } catch (...) {
        std::string errMsg = "unknown error";
        jp_result = reinterpret_cast<JpResult *>(::malloc(sizeof(JpResult) + errMsg.size() + 1));
        jp_result->flags = 1;
        std::memcpy(jp_result->content, errMsg.c_str(), errMsg.size());
        jp_result->content[errMsg.size()] = 0;
        return jp_result;
    }
    // muse be freed in JS
    jp_result = reinterpret_cast<JpResult *>(::malloc(sizeof(JpResult) + result.size() + 1));
    jp_result->flags = 0;
    jp_result->content[result.size()] = 0;
    std::memcpy(jp_result->content, result.c_str(), result.size());
    return jp_result;
}

EMSCRIPTEN_KEEPALIVE
JpResult *parse_to_ast(const char *str) {
    std::string content(str);
    ParserContext::Config config = ParserContext::Config::Default();
    auto ctx = std::make_shared<ParserContext>(-1, std::move(content), config);
    Parser parser(ctx);

    auto mod = parser.ParseModule();
    auto json = jetpack::dumper::AstToJson::Dump(mod);

    auto json_str = json.dump();

    // muse be freed in JS
    auto jp_result = reinterpret_cast<JpResult*>(::malloc(sizeof(JpResult) + json_str.size() + 1));
    jp_result->flags = 0;
    jp_result->content[json_str.size()] = 0;
    std::memcpy(jp_result->content, json_str.c_str(), json_str.size());
    return jp_result;
}

}
#endif
