//
// Created by Duzhong Chen on 2019/9/3.
//

#include "ast.h"

#define DD(NAME) \
    case AstNodeType::NAME: \
        return #NAME;

#define MARK(NAME) \
    if (NAME) { \
        marker(NAME); \
    }

#define MARK_VEC(NAME) \
    for (auto i : NAME) { \
        marker(i); \
    }

const char* AstNodeTypeToCString(AstNodeType t) {
    switch (t) {

        DEF_AST_NODE_TYPE(DD)

        default:
            return "<invalid>";
    }
}

const char* AstNode::TypeName() const {
    return AstNodeTypeToCString(type_);
}

#undef DD

#define DD(NAME) \
    NAME::NAME() { \
        type_ = AstNodeType::NAME; \
    }

DEF_AST_NODE_TYPE(DD)

#undef DD

void ArrayExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(elements_)
}

void ArrayPattern::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(elements_)
}

void ArrowFunctionExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(id_)
    MARK_VEC(params_)
    MARK(body_)
}

void AssignmentExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(left_)
    MARK(right_)
}

void AssignmentPattern::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(left_)
    MARK(right_)
}

void AsyncArrowFunctionExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(id_)
    MARK_VEC(params_)
    MARK(body_)
}

void AsyncFunctionDeclaration::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(id_)
    MARK_VEC(params_)
    MARK(body_)
}

void AsyncFunctionExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(id_)
    MARK_VEC(params_)
    MARK(body_)
}

void AwaitExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(arguments_)
}

void BinaryExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(left_)
    MARK(right_)
}

void BlockStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(body_)
}

void BreakStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(label_)
}

void CallExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(callee_)
    MARK_VEC(arguments_)
}

void CatchClause::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(param_)
    MARK(body_)
}

void ClassBody::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(body_)
}

void ClassDeclaration::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(id_)
    MARK(superClass_)
    MARK(body_)
}

void ClassExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(id_)
    MARK(superClass_)
    MARK(body_)
}

void ComputedMemberExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(object_)
    MARK(property_)
}

void ConditionalExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(test_)
    MARK(consequent_)
    MARK(alternate_)
}

void ContinueStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(label_)
}

void Directive::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(expression_)
}

void DoWhileStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(body_)
    MARK(test_)
}

void ExportAllDeclaration::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(source_)
}

void ExportDefaultDeclaration::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(declaration_)
}

void ExportNamedDeclaration::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(declaration_)
    MARK_VEC(specifiers_)
    MARK(source_)
}

void ExportSpecifier::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(exported_)
    MARK(local_)
}

void ExpressionStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(expression_)
}

void ForInStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(left_)
    MARK(right_)
    MARK(body_)
}

void ForOfStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(left_)
    MARK(right_)
    MARK(body_)
}

void ForStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(init_)
    MARK(test_)
    MARK(update_)
    MARK(body_)
}

void FunctionDeclaration::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(id_)
    MARK_VEC(params_)
    MARK(body_)
}

void FunctionExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(id_)
    MARK_VEC(params_)
    MARK(body_)
}

void IfStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(test_)
    MARK(consequent_)
    MARK(alternate_)
}

void ImportDeclaration::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(specifiers_)
    MARK(source_)
}

void ImportDefaultSpecifier::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(local_)
}

void ImportNamespaceSpecifier::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(local_)
}

void ImportSpecifier::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(local_)
    MARK(imported_)
}

void LabeledStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(label_)
    MARK(body_)
}

void MetaProperty::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(meta_)
    MARK(property_)
}

void MethodDefinition::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(key_)
    MARK(value_)
}

void Module::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(body_)
}

void NewExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(callee_)
    MARK_VEC(arguments_)
}

void ObjectExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(properties_)
}

void ObjectPattern::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(properties_)
}

void Property::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(key_)
    MARK(value_)
}

void RestElement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(argument_)
}

void ReturnStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(argument_)
}

void Script::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(body_)
}

void SequenceExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(expressions_)
}

void SpreadElement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(argument_)
}

void StaticMemberExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(object_)
    MARK(property_)
}

void SwitchCase::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(test_)
    MARK_VEC(consequent_)
}

void SwitchStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(discriminant_)
    MARK_VEC(cases_)
}

void TaggedTemplateExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(tag_)
    MARK(quasi_)
}

void TemplateLiteral::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(quasis_)
    MARK_VEC(expressions_)
}

void ThrowStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(argument_)
}

void TryStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(block_)
    MARK(handler_)
    MARK(finalizer_)
}

void UnaryExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(argument_)
}

void UpdateExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(argument_)
}

void VariableDeclaration::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK_VEC(declarations_)
}

void VariableDeclarator::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(id_)
    MARK(init_)
}

void WhileStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(test_)
    MARK(body_)
}

void WithStatement::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(object_)
    MARK(body_)
}

void YieldExpression::MarkChildren(GarbageCollector::MarkFunction marker) {
    MARK(argument_)
}

bool AstNode::IsArgumentListElement() const {
    return IsExpression() || type_ == AstNodeType::SpreadElement;
}

bool AstNode::IsArrayExpressionElement() const {
    return IsExpression() || type_ == AstNodeType::SpreadElement;
}

bool AstNode::IsArrayPatternElement() const {
    return type_ == AstNodeType::AssignmentPattern ||
        IsBindingIdentifier() ||
        IsBindingPattern() ||
        type_ == AstNodeType::RestElement;
}

bool AstNode::IsBindingPattern() const {
    return type_ == AstNodeType::ArrayPattern ||
     type_ == AstNodeType::ObjectPattern;
}

bool AstNode::IsBindingIdentifier() const {
    return type_ == AstNodeType::Identifier;
}

bool AstNode::IsExportableDefaultDeclaration() const {
    return IsBindingIdentifier() ||
        IsBindingPattern() ||
        type_ == AstNodeType::ClassDeclaration ||
        IsExpression() ||
        type_ == AstNodeType::FunctionDeclaration;
}

bool AstNode::IsExportableNamedDeclaration() const {
    return type_ == AstNodeType::AsyncFunctionDeclaration ||
        type_ == AstNodeType::ClassDeclaration ||
        type_ == AstNodeType::FunctionDeclaration ||
        type_ == AstNodeType::VariableDeclaration;
}

bool AstNode::IsExportDeclaration() const {
    return type_ == AstNodeType::ExportAllDeclaration ||
        type_ == AstNodeType::ExportDefaultDeclaration ||
        type_ == AstNodeType::ExportNamedDeclaration;
}

bool AstNode::IsFunctionParameter() const {
    return type_ == AstNodeType::AssignmentPattern ||
        IsBindingIdentifier() ||
        IsBindingPattern();
}

bool AstNode::IsImportDeclarationSpecifier() const {
    return type_ == AstNodeType::ImportDefaultSpecifier ||
        type_ == AstNodeType::ImportNamespaceSpecifier ||
        type_ == AstNodeType::ImportSpecifier;
}

bool AstNode::IsObjectExpressionProperty() const {
    return type_ == AstNodeType::Property ||
        type_ == AstNodeType::SpreadElement;
}

bool AstNode::IsObjectPatternProperty() const {
    return type_ == AstNodeType::Property ||
        type_ == AstNodeType::RestElement;
}

bool AstNode::IsPropertyKey() const {
    return type_ == AstNodeType::Identifier ||
        type_ == AstNodeType::Literal;
}

bool AstNode::IsPropertyValue() const {
    return type_ == AstNodeType::AssignmentPattern ||
        type_ == AstNodeType::AsyncFunctionExpression ||
        IsBindingIdentifier() ||
        IsBindingPattern() ||
        type_ == AstNodeType::FunctionExpression;
}

bool AstNode::IsStatementListItem() const {
    return IsDeclaration() || IsStatement();
}
