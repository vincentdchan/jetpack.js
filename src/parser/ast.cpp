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
