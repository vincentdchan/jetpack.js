//
// Created by Duzhong Chen on 2019/9/27.
//

#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <cinttypes>
#include "node_traverser.h"

class CodeGen: public NodeTraverser {
public:
    struct Config;

    CodeGen();
    CodeGen(Config config);

    bool TraverseBefore(const Sp<ArrayExpression> &node) override;

    bool TraverseBefore(const Sp<ArrayPattern> &node) override;

    bool TraverseBefore(const Sp<ArrowFunctionExpression> &node) override;

    bool TraverseBefore(const Sp<AssignmentExpression> &node) override;

    bool TraverseBefore(const Sp<AssignmentPattern> &node) override;

    bool TraverseBefore(const Sp<AwaitExpression> &node) override;

    bool TraverseBefore(const Sp<BinaryExpression> &node) override;

    bool TraverseBefore(const Sp<BlockStatement> &node) override;

    bool TraverseBefore(const Sp<BreakStatement> &node) override;

    bool TraverseBefore(const Sp<CallExpression> &node) override;

    bool TraverseBefore(const Sp<CatchClause> &node) override;

    bool TraverseBefore(const Sp<ClassBody> &node) override;

    bool TraverseBefore(const Sp<ClassDeclaration> &node) override;

    bool TraverseBefore(const Sp<ClassExpression> &node) override;

    bool TraverseBefore(const Sp<ComputedMemberExpression> &node) override;

    bool TraverseBefore(const Sp<ConditionalExpression> &node) override;

    bool TraverseBefore(const Sp<ContinueStatement> &node) override;

    bool TraverseBefore(const Sp<DebuggerStatement> &node) override;

    bool TraverseBefore(const Sp<Directive> &node) override;

    bool TraverseBefore(const Sp<DoWhileStatement> &node) override;

    bool TraverseBefore(const Sp<EmptyStatement> &node) override;

    bool TraverseBefore(const Sp<ExportAllDeclaration> &node) override;

    bool TraverseBefore(const Sp<ExportDefaultDeclaration> &node) override;

    bool TraverseBefore(const Sp<ExportNamedDeclaration> &node) override;

    bool TraverseBefore(const Sp<ExportSpecifier> &node) override;

    bool TraverseBefore(const Sp<ExpressionStatement> &node) override;

    bool TraverseBefore(const Sp<ForInStatement> &node) override;

    bool TraverseBefore(const Sp<ForOfStatement> &node) override;


    bool TraverseBefore(const Sp<ForStatement> &node) override;

    bool TraverseBefore(const Sp<FunctionDeclaration> &node) override;

    bool TraverseBefore(const Sp<FunctionExpression> &node) override;

    bool TraverseBefore(const Sp<Identifier> &node) override;

    bool TraverseBefore(const Sp<IfStatement> &node) override;

    bool TraverseBefore(const Sp<Import> &node) override;

    bool TraverseBefore(const Sp<ImportDeclaration> &node) override;

    bool TraverseBefore(const Sp<ImportDefaultSpecifier> &node) override;

    bool TraverseBefore(const Sp<ImportNamespaceSpecifier> &node) override;

    bool TraverseBefore(const Sp<ImportSpecifier> &node) override;

    bool TraverseBefore(const Sp<LabeledStatement> &node) override;

    bool TraverseBefore(const Sp<Literal> &node) override;

    bool TraverseBefore(const Sp<MetaProperty> &node) override;

    bool TraverseBefore(const Sp<MethodDefinition> &node) override;

    bool TraverseBefore(const Sp<Module> &node) override;

    bool TraverseBefore(const Sp<NewExpression> &node) override;

    bool TraverseBefore(const Sp<ObjectExpression> &node) override;

    bool TraverseBefore(const Sp<ObjectPattern> &node) override;

    bool TraverseBefore(const Sp<Property> &node) override;

    bool TraverseBefore(const Sp<RegexLiteral> &node) override;

    bool TraverseBefore(const Sp<RestElement> &node) override;

    bool TraverseBefore(const Sp<ReturnStatement> &node) override;

    bool TraverseBefore(const Sp<Script> &node) override;

    bool TraverseBefore(const Sp<SequenceExpression> &node) override;

    bool TraverseBefore(const Sp<SpreadElement> &node) override;

    bool TraverseBefore(const Sp<StaticMemberExpression> &node) override;

    bool TraverseBefore(const Sp<Super> &node) override;

    bool TraverseBefore(const Sp<SwitchCase> &node) override;

    bool TraverseBefore(const Sp<SwitchStatement> &node) override;

    bool TraverseBefore(const Sp<TaggedTemplateExpression> &node) override;

    bool TraverseBefore(const Sp<TemplateElement> &node) override;

    bool TraverseBefore(const Sp<TemplateLiteral> &node) override;

    bool TraverseBefore(const Sp<ThisExpression> &node) override;

    bool TraverseBefore(const Sp<ThrowStatement> &node) override;

    bool TraverseBefore(const Sp<TryStatement> &node) override;

    bool TraverseBefore(const Sp<UnaryExpression> &node) override;

    bool TraverseBefore(const Sp<UpdateExpression> &node) override;

    bool TraverseBefore(const Sp<VariableDeclaration> &node) override;

    bool TraverseBefore(const Sp<VariableDeclarator> &node) override;

    bool TraverseBefore(const Sp<WhileStatement> &node) override;

    bool TraverseBefore(const Sp<WithStatement> &node) override;

    bool TraverseBefore(const Sp<YieldExpression> &node) override;

    bool TraverseBefore(const Sp<ArrowParameterPlaceHolder> &node) override;

private:
    class State;

    std::unique_ptr<State> state;

};

struct CodeGen::Config {
    std::uint32_t start_indent_level = 0;
    std::uint32_t indent_level = 2;
    bool source_map = false;
    bool comments = true;

};

class CodeGen::State {
public:

    State() = default;

    State(const State&) = delete;
    State& operator=(const State&) = delete;

    State(State&& state) = default;
    State& operator=(State&&) = default;

    std::int64_t line = 1;
    std::int64_t column = 0;
    std::stringstream output;

    void Write(const std::string& content);

};
