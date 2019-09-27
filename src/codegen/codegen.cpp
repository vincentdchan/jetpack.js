//
// Created by Duzhong Chen on 2019/9/27.
//

#include "codegen.h"

using namespace std;

CodeGen::CodeGen() {
    CodeGen(Config());
}

CodeGen::CodeGen(Config config) {
    state = make_unique<CodeGen::State>();
}

bool CodeGen::TraverseBefore(const Sp<ArrayExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ArrayPattern> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ArrowFunctionExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<AssignmentExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<AssignmentPattern> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<AwaitExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<BinaryExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<BlockStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<BreakStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<CallExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<CatchClause> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ClassBody> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ClassDeclaration> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ClassExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ComputedMemberExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ConditionalExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ContinueStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<DebuggerStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<Directive> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<DoWhileStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<EmptyStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ExportAllDeclaration> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ExportDefaultDeclaration> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ExportNamedDeclaration> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ExportSpecifier> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ExpressionStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ForInStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ForOfStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ForStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<FunctionDeclaration> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<FunctionExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<Identifier> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<IfStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<Import> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ImportDeclaration> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ImportDefaultSpecifier> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ImportNamespaceSpecifier> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ImportSpecifier> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<LabeledStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<Literal> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<MetaProperty> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<MethodDefinition> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<Module> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<NewExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ObjectExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ObjectPattern> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<Property> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<RegexLiteral> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<RestElement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ReturnStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<Script> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<SequenceExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<SpreadElement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<StaticMemberExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<Super> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<SwitchCase> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<SwitchStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<TaggedTemplateExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<TemplateElement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<TemplateLiteral> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ThisExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ThrowStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<TryStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<UnaryExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<UpdateExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<VariableDeclaration> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<VariableDeclarator> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<WhileStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<WithStatement> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<YieldExpression> &node) {
    return true;
}

bool CodeGen::TraverseBefore(const Sp<ArrowParameterPlaceHolder> &node) {
    return true;
}

