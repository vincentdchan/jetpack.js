//
// Created by Duzhong Chen on 2019/9/3.
//

#include "ast.h"

#define DD(NAME) \
    case AstNodeType::NAME: \
        return #NAME;

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

void ArrayExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(elements_);
}

void ArrayPattern::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(elements_);
}

void ArrowFunctionExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(id_);
    mark(params_);
    mark(body_);
}

void AssignmentExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(left_);
    mark(right_);
}

void AssignmentPattern::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(left_);
    mark(right_);
}

void AsyncArrowFunctionExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(id_);
    mark(params_);
    mark(body_);
}

void AsyncFunctionDeclaration::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(id_);
    mark(params_);
    mark(body_);
}

void AsyncFunctionExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(id_);
    mark(params_);
    mark(body_);
}

void AwaitExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(arguments_);
}

void BinaryExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(left_);
    mark(right_);
}

void BlockStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(body_);
}

void BreakStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(label_);
}

void CallExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(callee_);
    mark(arguments_);
}

void CatchClause::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(param_);
    mark(body_);
}

void ClassBody::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(body_);
}

void ClassDeclaration::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(id_);
    mark(superClass_);
    mark(body_);
}

void ClassExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(id_);
    mark(superClass_);
    mark(body_);
}

void ComputedMemberExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(object_);
    mark(property_);
}

void ConditionalExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(test_);
    mark(consequent_);
    mark(alternate_);
}

void ContinueStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(label_);
}

void Directive::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(expression_);
}

void DoWhileStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(body_);
    mark(test_);
}

void ExportAllDeclaration::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(source_);
}

void ExportDefaultDeclaration::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(declaration_);
}

void ExportNamedDeclaration::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(declaration_);
    mark(specifiers_);
    mark(source_);
}

void ExportSpecifier::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(exported_);
    mark(local_);
}

void ExpressionStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(expression_);
}

void ForInStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(left_);
    mark(right_);
    mark(body_);
}

void ForOfStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(left_);
    mark(right_);
    mark(body_);
}

void ForStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(init_);
    mark(test_);
    mark(update_);
    mark(body_);
}

void FunctionDeclaration::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(id_);
    mark(params_);
    mark(body_);
}

void FunctionExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(id_);
    mark(params_);
    mark(body_);
}

void IfStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(test_);
    mark(consequent_);
    mark(alternate_);
}

void ImportDeclaration::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(specifiers_);
    mark(source_);
}

void ImportDefaultSpecifier::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(local_);
}

void ImportNamespaceSpecifier::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(local_);
}

void ImportSpecifier::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(local_);
    mark(imported_);
}

void LabeledStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(label_);
    mark(body_);
}

void MetaProperty::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(meta_);
    mark(property_);
}

void MethodDefinition::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(key_);
    mark(value_);
}

void Module::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(body_);
}

void NewExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(callee_);
    mark(arguments_);
}

void ObjectExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(properties_);
}

void ObjectPattern::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(properties_);
}

void Property::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(key_);
    mark(value_);
}

void RestElement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(argument_);
}

void ReturnStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(argument_);
}

void Script::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(body_);
}

void SequenceExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(expressions_);
}

void SpreadElement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(argument_);
}

void StaticMemberExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(object_);
    mark(property_);
}

void SwitchCase::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(test_);
    mark(consequent_);
}

void SwitchStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(discriminant_);
    mark(cases_);
}

void TaggedTemplateExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(tag_);
    mark(quasi_);
}

void TemplateLiteral::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(quasis_);
    mark(expressions_);
}

void ThrowStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(argument_);
}

void TryStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(block_);
    mark(handler_);
    mark(finalizer_);
}

void UnaryExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(argument_);
}

void UpdateExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(argument_);
}

void VariableDeclaration::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(declarations_);
}

void VariableDeclarator::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(id_);
    mark(init_);
}

void WhileStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(test_);
    mark(body_);
}

void WithStatement::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(object_);
    mark(body_);
}

void YieldExpression::MarkChildren(GarbageCollector::MarkFunction mark) {
    mark(argument_);
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
