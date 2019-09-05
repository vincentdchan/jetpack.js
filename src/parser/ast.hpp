//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <vector>
#include <string>
#include <variant>
#include <optional>
#include "../tokenizer/token.h"
#include "../macros.h"

#define DEF_AST_NODE_TYPE(D) \
    D(ArrayExpression) \
    D(ArrayPattern) \
    D(ArrowFunctionExpression) \
    D(AssignmentExpression) \
    D(AssignmentPattern) \
    D(AsyncArrowFunctionExpression) \
    D(AsyncFunctionDeclaration) \
    D(AsyncFunctionExpression) \
    D(AwaitExpression) \
    D(BinaryExpression) \
    D(BlockStatement) \
    D(BreakStatement) \
    D(CallExpression) \
    D(CatchClause) \
    D(ClassBody) \
    D(ClassDeclaration) \
    D(ClassExpression) \
    D(ComputedMemberExpression) \
    D(ConditionalExpression) \
    D(ContinueStatement) \
    D(DebuggerStatement) \
    D(Directive) \
    D(DoWhileStatement) \
    D(EmptyStatement) \
    D(ExportAllDeclaration) \
    D(ExportDefaultDeclaration) \
    D(ExportNamedDeclaration) \
    D(ExportSpecifier) \
    D(ExpressionStatement) \
    D(ForInStatement) \
    D(ForOfStatement) \
    D(ForStatement) \
    D(FunctionDeclaration) \
    D(FunctionExpression) \
    D(Identifier) \
    D(IfStatement) \
    D(Import) \
    D(ImportDeclaration) \
    D(ImportDefaultSpecifier) \
    D(ImportNamespaceSpecifier) \
    D(ImportSpecifier) \
    D(LabeledStatement) \
    D(Literal) \
    D(MetaProperty) \
    D(MethodDefinition) \
    D(Module) \
    D(NewExpression) \
    D(ObjectExpression) \
    D(ObjectPattern) \
    D(Property) \
    D(RegexLiteral) \
    D(RestElement) \
    D(ReturnStatement) \
    D(Script) \
    D(SequenceExpression) \
    D(SpreadElement) \
    D(StaticMemberExpression) \
    D(Super) \
    D(SwitchCase) \
    D(SwitchStatement) \
    D(TaggedTemplateExpression) \
    D(TemplateElement) \
    D(TemplateLiteral) \
    D(ThisExpression) \
    D(ThrowStatement) \
    D(TryStatement) \
    D(UnaryExpression) \
    D(UpdateExpression) \
    D(VariableDeclaration) \
    D(VariableDeclarator) \
    D(WhileStatement) \
    D(WithStatement) \
    D(YieldExpression)


#define DD(name) \
template <typename T> \
    class name;


DEF_AST_NODE_TYPE(DD)

#undef DD

template <typename T>
class Expression;

template <typename T>
using BindingIdentifier = Identifier<T>;

#define DD(name) name,

enum class AstNodeType: std::uint8_t {
    Invalid,

    DEF_AST_NODE_TYPE(DD)
};

#undef DD

enum class VarKind {
    Invalid = 0,
    Var,
    Let,
    Const,
};

static const char* AstNodeTypeToCString(AstNodeType t);

template<typename T>
class AstNode {
public:

    AstNodeType type_ = AstNodeType::Invalid;
    T annot_;

//    AstNode(): type_(AstNodeType::Invalid) {}

    virtual bool IsArgumentListElement() const;
    virtual bool IsArrayExpressionElement() const;
    virtual bool IsArrayPatternElement() const;
    virtual bool IsBindingPattern() const;
    virtual bool IsBindingIdentifier() const;
    virtual bool IsExportableDefaultDeclaration() const;
    virtual bool IsExportableNamedDeclaration() const;
    virtual bool IsExportDeclaration() const;
    virtual bool IsFunctionParameter() const;
    virtual bool IsImportDeclarationSpecifier() const;
    virtual bool IsObjectExpressionProperty() const;
    virtual bool IsObjectPatternProperty() const;
    virtual bool IsPropertyKey() const;
    virtual bool IsPropertyValue() const;
    virtual bool IsStatementListItem() const;

    virtual bool IsDeclaration() const { return false; }
    virtual bool IsExpression() const { return false; }
    virtual bool IsStatement() const { return false; }

    const char* TypeName() const;

    ~AstNode() = default;
};


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

template <typename T>
const char* AstNode<T>::TypeName() const {
    return AstNodeTypeToCString(type_);
}

#undef DD

template <typename T>
bool AstNode<T>::IsArgumentListElement() const {
    return IsExpression() || type_ == AstNodeType::SpreadElement;
}

template <typename T>
bool AstNode<T>::IsArrayExpressionElement() const {
    return IsExpression() || type_ == AstNodeType::SpreadElement;
}

template <typename T>
bool AstNode<T>::IsArrayPatternElement() const {
    return type_ == AstNodeType::AssignmentPattern ||
           IsBindingIdentifier() ||
           IsBindingPattern() ||
           type_ == AstNodeType::RestElement;
}

template <typename T>
bool AstNode<T>::IsBindingPattern() const {
    return type_ == AstNodeType::ArrayPattern ||
           type_ == AstNodeType::ObjectPattern;
}

template <typename T>
bool AstNode<T>::IsBindingIdentifier() const {
    return type_ == AstNodeType::Identifier;
}

template <typename T>
bool AstNode<T>::IsExportableDefaultDeclaration() const {
    return IsBindingIdentifier() ||
           IsBindingPattern() ||
           type_ == AstNodeType::ClassDeclaration ||
           IsExpression() ||
           type_ == AstNodeType::FunctionDeclaration;
}

template <typename T>
bool AstNode<T>::IsExportableNamedDeclaration() const {
    return type_ == AstNodeType::AsyncFunctionDeclaration ||
           type_ == AstNodeType::ClassDeclaration ||
           type_ == AstNodeType::FunctionDeclaration ||
           type_ == AstNodeType::VariableDeclaration;
}

template <typename T>
bool AstNode<T>::IsExportDeclaration() const {
    return type_ == AstNodeType::ExportAllDeclaration ||
           type_ == AstNodeType::ExportDefaultDeclaration ||
           type_ == AstNodeType::ExportNamedDeclaration;
}

template <typename T>
bool AstNode<T>::IsFunctionParameter() const {
    return type_ == AstNodeType::AssignmentPattern ||
           IsBindingIdentifier() ||
           IsBindingPattern();
}

template <typename T>
bool AstNode<T>::IsImportDeclarationSpecifier() const {
    return type_ == AstNodeType::ImportDefaultSpecifier ||
           type_ == AstNodeType::ImportNamespaceSpecifier ||
           type_ == AstNodeType::ImportSpecifier;
}

template <typename T>
bool AstNode<T>::IsObjectExpressionProperty() const {
    return type_ == AstNodeType::Property ||
           type_ == AstNodeType::SpreadElement;
}

template <typename T>
bool AstNode<T>::IsObjectPatternProperty() const {
    return type_ == AstNodeType::Property ||
           type_ == AstNodeType::RestElement;
}

template <typename T>
bool AstNode<T>::IsPropertyKey() const {
    return type_ == AstNodeType::Identifier ||
           type_ == AstNodeType::Literal;
}

template <typename T>
bool AstNode<T>::IsPropertyValue() const {
    return type_ == AstNodeType::AssignmentPattern ||
           type_ == AstNodeType::AsyncFunctionExpression ||
           IsBindingIdentifier() ||
           IsBindingPattern() ||
           type_ == AstNodeType::FunctionExpression;
}

template <typename T>
bool AstNode<T>::IsStatementListItem() const {
    return IsDeclaration() || IsStatement();
}

template <typename T>
class Expression: public AstNode<T> {
public:
    bool IsExpression() const override { return true; }

};

template <typename T>
class Statement: public AstNode<T> {
public:
    bool IsStatement() const override { return true; }

};

template <typename T>
class Declaration: public AstNode<T> {
public:
    bool IsDeclaration() const override { return true; }

};

template <typename  T>
class ArrayExpression: public Expression<T> {
public:

    ArrayExpression();

    std::vector<Sp<AstNode<T>>> elements_;

};

template <typename T>
class ArrayPattern: public AstNode<T> {
public:
    ArrayPattern();

    std::vector<Sp<AstNode<T>>> elements_;

};

template <typename T>
class ArrowFunctionExpression: public Expression<T> {
public:
    ArrowFunctionExpression();

    Sp<Identifier<T>> id_;
    std::vector<Sp<AstNode<T>>> params_;
    Sp<AstNode<T>> body_;
    // TODO: flags
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

template <typename T>
class AssignmentExpression: public Expression<T> {
public:
    AssignmentExpression();

    UString operator_;
    Sp<Expression<T>> left_;
    Sp<Expression<T>> right_;

};

template <typename T>
class AssignmentPattern: public AstNode<T> {
public:
    AssignmentPattern();

    Sp<AstNode<T>> left_;
    Sp<Expression<T>> right_;

};

template <typename T>
class AsyncArrowFunctionExpression: public Expression<T> {
public:
    AsyncArrowFunctionExpression();

    Sp<Identifier<T>> id_;
    std::vector<Sp<AstNode<T>>> params_;
    Sp<AstNode<T>> body_;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

template <typename T>
class AsyncFunctionDeclaration: public Declaration<T> {
public:
    AsyncFunctionDeclaration();

    Sp<Identifier<T>> id_;
    std::vector<Sp<AstNode<T>>> params_;
    Sp<BlockStatement<T>> body_;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

template <typename T>
class AsyncFunctionExpression: public Expression<T> {
public:
    AsyncFunctionExpression();

    Sp<Identifier<T>> id_;
    std::vector<Sp<AstNode<T>>> params_;
    Sp<BlockStatement<T>> body_;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

template <typename T>
class AwaitExpression: public Expression<T> {
public:
    AwaitExpression();

    Sp<Expression<T>> arguments_;

};

template <typename T>
class BinaryExpression: public Expression<T> {
public:
    BinaryExpression();

    UString operator_;
    Sp<Expression<T>> left_;
    Sp<Expression<T>> right_;

};

template <typename T>
class BlockStatement: public AstNode<T> {
public:
    BlockStatement();

    std::vector<Sp<Statement<T>>> body_;

};

template <typename T>
class BreakStatement: public Statement<T> {
public:
    BreakStatement();

    Sp<Identifier<T>> label_;

};

template <typename T>
class CallExpression: public Expression<T> {
public:
    CallExpression();

    Sp<AstNode<T>> callee_;
    std::vector<Sp<AstNode<T>>> arguments_;

};

template <typename T>
class CatchClause: public AstNode<T> {
public:
    CatchClause();

    Sp<AstNode<T>> param_;
    Sp<BlockStatement<T>> body_;

};

template <typename T>
class ClassBody: public AstNode<T> {
public:
    ClassBody();

    std::vector<Sp<Property<T>>> body_;

};

template <typename T>
class ClassDeclaration: public Declaration<T> {
public:
    ClassDeclaration();

    Sp<Identifier<T>> id_;
    Sp<Identifier<T>> superClass_;
    Sp<ClassBody<T>> body_;

};

template <typename T>
class ClassExpression: public Expression<T> {
public:
    ClassExpression();

    Sp<Identifier<T>> id_;
    Sp<Identifier<T>> superClass_;
    Sp<ClassBody<T>> body_;

};

template <typename T>
class ComputedMemberExpression: public Expression<T> {
public:
    ComputedMemberExpression();

    bool computed_ = false;
    Sp<Expression<T>> object_;
    Sp<Expression<T>> property_;

};

template <typename T>
class ConditionalExpression: public Expression<T> {
public:
    ConditionalExpression();

    Sp<Expression<T>> test_;
    Sp<Expression<T>> consequent_;
    Sp<Expression<T>> alternate_;

};

template <typename T>
class ContinueStatement: public Statement<T> {
public:
    ContinueStatement();

    Sp<Identifier<T>> label_;

};

template <typename T>
class DebuggerStatement: public Statement<T> {
public:
    DebuggerStatement();

};

template <typename T>
class Directive: public Statement<T> {
public:
    Directive();

    Sp<Expression<T>> expression_;
    UString directive_;

};

template <typename T>
class DoWhileStatement: public Statement<T> {
public:
    DoWhileStatement();

    Sp<Statement<T>> body_;
    Sp<Expression<T>> test_;

};

template <typename T>
class EmptyStatement: public Statement<T> {
public:
    EmptyStatement();

};

template <typename T>
class ExportAllDeclaration: public Declaration<T> {
public:
    ExportAllDeclaration();

    Sp<Literal<T>> source_;

};

template <typename T>
class ExportDefaultDeclaration: public Declaration<T> {
public:
    ExportDefaultDeclaration();

    Sp<AstNode<T>> declaration_;

};

template <typename T>
class ExportNamedDeclaration: public Declaration<T> {
public:
    ExportNamedDeclaration();

    std::vector<Sp<AstNode<T>>> declaration_;
    std::vector<Sp<ExportSpecifier<T>>> specifiers_;
    Sp<Literal<T>> source_;

};

template <typename T>
class ExportSpecifier: public AstNode<T> {
public:
    ExportSpecifier();

    Sp<Identifier<T>> exported_;
    Sp<Identifier<T>> local_;

};

template <typename T>
class ExpressionStatement: public Statement<T> {
public:
    ExpressionStatement();

    Sp<Expression<T>> expression_;

};

template <typename T>
class ForInStatement: public Statement<T> {
public:
    ForInStatement();

    Sp<Expression<T>> left_;
    Sp<Expression<T>> right_;
    Sp<Statement<T>> body_;
    bool each_ = false;

};

template <typename T>
class ForOfStatement: public Statement<T> {
public:
    ForOfStatement();

    Sp<Expression<T>> left_;
    Sp<Expression<T>> right_;
    Sp<Statement<T>> body_;

};

template <typename T>
class ForStatement: public Statement<T> {
public:
    ForStatement();

    Sp<Expression<T>> init_;
    Sp<Expression<T>> test_;
    Sp<Expression<T>> update_;
    Sp<Statement<T>> body_;

};

template <typename T>
class FunctionDeclaration: public Declaration<T> {
public:
    FunctionDeclaration();

    Sp<Identifier<T>> id_;
    std::vector<Sp<AstNode<T>>> params_;
    Sp<BlockStatement<T>> body_;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

template <typename T>
class FunctionExpression: public Expression<T> {
public:
    FunctionExpression();

    Sp<Identifier<T>> id_;
    std::vector<Sp<AstNode<T>>> params_;
    Sp<BlockStatement<T>> body_;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

template <typename T>
class Identifier: public Expression<T> {
public:
    Identifier();

    UString name_;

};

template <typename T>
class IfStatement: public Statement<T> {
public:
    IfStatement();

    Sp<Expression<T>> test_;
    Sp<Statement<T>> consequent_;
    Sp<Statement<T>> alternate_;

};

template <typename T>
class Import: public AstNode<T> {
public:
    Import();

};

template <typename T>
class ImportDeclaration: public Declaration<T> {
public:
    ImportDeclaration();

    std::vector<Sp<AstNode<T>>> specifiers_;
    Sp<Literal<T>> source_;

};

template <typename T>
class ImportDefaultSpecifier: public AstNode<T> {
public:
    ImportDefaultSpecifier();

    Sp<Identifier<T>> local_;

};

template <typename T>
class ImportNamespaceSpecifier: public AstNode<T> {
public:
    ImportNamespaceSpecifier();

    Sp<Identifier<T>> local_;

};

template <typename T>
class ImportSpecifier: public AstNode<T> {
public:
    ImportSpecifier();

    Sp<Identifier<T>> local_;
    Sp<Identifier<T>> imported_;

};

template <typename T>
class LabeledStatement: public AstNode<T> {
public:
    LabeledStatement();

    Sp<Identifier<T>> label_;
    Sp<Statement<T>> body_;

};

template <typename T>
class Literal: public Expression<T> {
public:
    Literal();

    std::optional<std::variant<bool, JS_Number, UString>> value_;
    UString raw_;

};

template <typename T>
class MetaProperty: public AstNode<T> {
public:
    MetaProperty();

    Sp<Identifier<T>> meta_;
    Sp<Identifier<T>> property_;

};

template <typename T>
class MethodDefinition: public AstNode<T> {
public:
    MethodDefinition();

    Sp<Expression<T>> key_;
    bool computed_ = false;
    Sp<AstNode<T>> value_;
    VarKind kind_ = VarKind::Invalid;
    bool static_ = false;

};

template <typename T>
class Module: public AstNode<T> {
public:
    Module();

    std::vector<Sp<AstNode<T>>> body_;
    UString sourceType_;

};

template <typename T>
class NewExpression: public Expression<T> {
public:
    NewExpression();

    Sp<Expression<T>> callee_;
    std::vector<Sp<AstNode<T>>> arguments_;

};

template <typename T>
class ObjectExpression: public Expression<T> {
public:
    ObjectExpression();

    std::vector<Sp<AstNode<T>>> properties_;

};

template <typename T>
class ObjectPattern: public AstNode<T> {
public:
    ObjectPattern();

    std::vector<Sp<AstNode<T>>> properties_;

};

template <typename T>
class Property: public AstNode<T> {
public:
    Property();

    Sp<AstNode<T>> key_;
    bool computed_ = false;
    Sp<AstNode<T>> value_;
    VarKind kind_ = VarKind::Invalid;
    bool method_ = false;
    bool shorthand_ = false;

};

template <typename T>
class RegexLiteral: public Expression<T> {
public:
    RegexLiteral();

    JS_RegExp value_;
    UString raw_;

};

template <typename T>
class RestElement: public AstNode<T> {
public:
    RestElement();

    Sp<AstNode<T>> argument_;

};

template <typename T>
class ReturnStatement: public Statement<T> {
public:
    ReturnStatement();

    Sp<Expression<T>> argument_;

};

template <typename T>
class Script: public AstNode<T> {
public:
    Script();

    std::vector<Sp<AstNode<T>>> body_;
    UString sourceType_;

};

template <typename T>
class SequenceExpression: public Expression<T> {
public:
    SequenceExpression();

    std::vector<Sp<Expression<T>>> expressions_;

};

template <typename T>
class SpreadElement: public AstNode<T> {
public:
    SpreadElement();

    Sp<Expression<T>> argument_;

};

template <typename T>
class StaticMemberExpression: public Expression<T> {
public:
    StaticMemberExpression();

    bool computed_ = false;
    Sp<Expression<T>> object_;
    Sp<Expression<T>> property_;

};

template <typename T>
class Super: public AstNode<T> {
public:
    Super();

};

template <typename T>
class SwitchCase: public AstNode<T> {
public:
    SwitchCase();

    Sp<Expression<T>> test_;
    std::vector<Sp<Statement<T>>> consequent_;

};

template <typename T>
class SwitchStatement: public Statement<T> {
public:
    SwitchStatement();

    Sp<Expression<T>> discriminant_;
    std::vector<Sp<SwitchCase<T>>> cases_;

};

template <typename T>
class TaggedTemplateExpression: public Expression<T> {
public:
    TaggedTemplateExpression();

    Sp<Expression<T>> tag_;
    Sp<TemplateLiteral<T>> quasi_;

};

class TemplateElementValue {
public:
    TemplateElementValue() : raw_(UString()) {}

    UString cooked_;
    UString raw_;

};

template <typename T>
class TemplateElement: public AstNode<T> {
public:
    TemplateElement();

    TemplateElementValue value_;
    bool tail_ = false;

};

template <typename T>
class TemplateLiteral: public AstNode<T> {
public:
    TemplateLiteral();

    std::vector<Sp<TemplateElement<T>>> quasis_;
    std::vector<Sp<Expression<T>>> expressions_;

};

template <typename T>
class ThisExpression: public Expression<T> {
public:
    ThisExpression();

};

template <typename T>
class ThrowStatement: public Statement<T> {
public:
    ThrowStatement();

    Sp<Expression<T>> argument_;

};

template <typename T>
class TryStatement: public Statement<T> {
public:
    TryStatement();

    Sp<BlockStatement<T>> block_;
    Sp<CatchClause<T>> handler_;
    Sp<BlockStatement<T>> finalizer_;

};

template <typename T>
class UnaryExpression: public Expression<T> {
public:
    UnaryExpression();

    UString operator_;
    Sp<Expression<T>> argument_;
    bool prefix_ = false;

};

template <typename T>
class UpdateExpression: public Expression<T> {
public:
    UpdateExpression();

    UString operator_;
    Sp<Expression<T>> argument_;
    bool prefix_ = false;

};

template <typename T>
class VariableDeclaration: public Declaration<T> {
public:
    VariableDeclaration();

    std::vector<Sp<VariableDeclarator<T>>> declarations_;
    VarKind kind_ = VarKind::Invalid;

};

template <typename T>
class VariableDeclarator: public AstNode<T> {
public:
    VariableDeclarator();

    Sp<AstNode<T>> id_;
    Sp<Expression<T>> init_;

};

template <typename T>
class WhileStatement: public Statement<T> {
public:
    WhileStatement();

    Sp<Expression<T>> test_;
    Sp<Statement<T>> body_;

};

template <typename T>
class WithStatement: public Statement<T> {
public:
    WithStatement();

    Sp<Expression<T>> object_;
    Sp<Statement<T>> body_;

};

template <typename T>
class YieldExpression: public Expression<T> {
public:
    YieldExpression();

    Sp<Expression<T>> argument_;
    bool delegate_ = false;

};

#define DD(NAME) \
    template <typename T> \
    NAME<T>::NAME() { \
        AstNode<T>::type_ = AstNodeType::NAME; \
    }

DEF_AST_NODE_TYPE(DD)

#undef DD

