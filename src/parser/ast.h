//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <vector>
#include <string>
#include <variant>
#include <optional>
#include "../tokenizer/token.h"

typedef double JS_Number;
typedef UString JS_RegExp;

template <typename T>
using Sp = std::shared_ptr<T>;

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
    class name;


DEF_AST_NODE_TYPE(DD)

#undef DD

class Expression;

typedef Identifier BindingIdentifier;

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

class AstNode {
public:

    AstNodeType type_;
    std::pair<std::uint32_t, std::uint32_t> range_;
    SourceLocation loc_;

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

class Expression: public AstNode {
public:
    bool IsExpression() const override { return true; }

};

class Statement: public AstNode {
public:
    bool IsStatement() const override { return true; }
};

class Declaration: public AstNode {
public:
    bool IsDeclaration() const override { return true; }

};

class ArrayExpression: public Expression {
public:

    ArrayExpression();

    std::vector<Sp<AstNode>> elements_;

};

class ArrayPattern: public AstNode {
public:
    ArrayPattern();

    std::vector<Sp<AstNode>> elements_;

};

class ArrowFunctionExpression: public Expression {
public:
    ArrowFunctionExpression();

    Sp<Identifier> id_;
    std::vector<Sp<AstNode>> params_;
    Sp<AstNode> body_;
    // TODO: flags
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

class AssignmentExpression: public Expression {
public:
    AssignmentExpression();

    UString operator_;
    Sp<Expression> left_;
    Sp<Expression> right_;

};

class AssignmentPattern: public AstNode {
public:
    AssignmentPattern();

    Sp<AstNode> left_;
    Sp<Expression> right_;
};

class AsyncArrowFunctionExpression: public Expression {
public:
    AsyncArrowFunctionExpression();

    Sp<Identifier> id_;
    std::vector<Sp<AstNode>> params_;
    Sp<AstNode> body_;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;
};

class AsyncFunctionDeclaration: public Declaration {
public:
    AsyncFunctionDeclaration();

    Sp<Identifier> id_;
    std::vector<Sp<AstNode>> params_;
    Sp<BlockStatement> body_;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;
};

class AsyncFunctionExpression: public Expression {
public:
    AsyncFunctionExpression();

    Sp<Identifier> id_;
    std::vector<Sp<AstNode>> params_;
    Sp<BlockStatement> body_;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

class AwaitExpression: public Expression {
public:
    AwaitExpression();

    Sp<Expression> arguments_;

};

class BinaryExpression: public Expression {
public:
    BinaryExpression();

    UString operator_;
    Sp<Expression> left_;
    Sp<Expression> right_;

};

class BlockStatement: public AstNode {
public:
    BlockStatement();

    std::vector<Sp<Statement>> body_;

};

class BreakStatement: public Statement {
public:
    BreakStatement();

    Sp<Identifier> label_;

};

class CallExpression: public Expression {
public:
    CallExpression();

    Sp<AstNode> callee_;
    std::vector<Sp<AstNode>> arguments_;

};

class CatchClause: public AstNode {
public:
    CatchClause();

    Sp<AstNode> param_;
    Sp<BlockStatement> body_;

};

class ClassBody: public AstNode {
public:
    ClassBody();

    std::vector<Sp<Property>> body_;

};

class ClassDeclaration: public Declaration {
public:
    ClassDeclaration();

    Sp<Identifier> id_;
    Sp<Identifier> superClass_;
    Sp<ClassBody> body_;

};

class ClassExpression: public Expression {
public:
    ClassExpression();

    Sp<Identifier> id_;
    Sp<Identifier> superClass_;
    Sp<ClassBody> body_;

};

class ComputedMemberExpression: public Expression {
public:
    ComputedMemberExpression();

    bool computed_ = false;
    Sp<Expression> object_;
    Sp<Expression> property_;

};

class ConditionalExpression: public Expression {
public:
    ConditionalExpression();

    Sp<Expression> test_;
    Sp<Expression> consequent_;
    Sp<Expression> alternate_;

};

class ContinueStatement: public Statement {
public:
    ContinueStatement();

    Sp<Identifier> label_ = nullptr;

};

class DebuggerStatement: public Statement {
public:
    DebuggerStatement();

};

class Directive: public Statement {
public:
    Directive();

    Sp<Expression> expression_;
    UString directive_;

};

class DoWhileStatement: public Statement {
public:
    DoWhileStatement();

    Sp<Statement> body_;
    Sp<Expression> test_;

};

class EmptyStatement: public Statement {
public:
    EmptyStatement();

};

class ExportAllDeclaration: public Declaration {
public:
    ExportAllDeclaration();

    Sp<Literal> source_;

};

class ExportDefaultDeclaration: public Declaration {
public:
    ExportDefaultDeclaration();

    Sp<AstNode> declaration_;

};

class ExportNamedDeclaration: public Declaration {
public:
    ExportNamedDeclaration();

    std::vector<Sp<AstNode>> declaration_;
    std::vector<Sp<ExportSpecifier>> specifiers_;
    Sp<Literal> source_;

};

class ExportSpecifier: public AstNode {
public:
    ExportSpecifier();

    Sp<Identifier> exported_;
    Sp<Identifier> local_;

};

class ExpressionStatement: public Statement {
public:
    ExpressionStatement();

    Sp<Expression> expression_;

};

class ForInStatement: public Statement {
public:
    ForInStatement();

    Sp<Expression> left_;
    Sp<Expression> right_;
    Sp<Statement> body_;
    bool each_ = false;

};

class ForOfStatement: public Statement {
public:
    ForOfStatement();

    Sp<Expression> left_;
    Sp<Expression> right_;
    Sp<Statement> body_;

};

class ForStatement: public Statement {
public:
    ForStatement();

    Sp<Expression> init_;
    Sp<Expression> test_;
    Sp<Expression> update_;
    Sp<Statement> body_;

};

class FunctionDeclaration: public Declaration {
public:
    FunctionDeclaration();

    Sp<Identifier> id_;
    std::vector<Sp<AstNode>> params_;
    Sp<BlockStatement> body_;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

class FunctionExpression: public Expression {
public:
    FunctionExpression();

    Sp<Identifier> id_;
    std::vector<Sp<AstNode>> params_;
    Sp<BlockStatement> body_;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

class Identifier: public Expression {
public:
    Identifier();

    UString name_;

};

class IfStatement: public Statement {
public:
    IfStatement();

    Sp<Expression> test_;
    Sp<Statement> consequent_;
    Sp<Statement> alternate_;

};

class Import: public AstNode {
public:
    Import();

};

class ImportDeclaration: public Declaration {
public:
    ImportDeclaration();

    std::vector<Sp<AstNode>> specifiers_;
    Sp<Literal> source_;

};

class ImportDefaultSpecifier: public AstNode {
public:
    ImportDefaultSpecifier();

    Sp<Identifier> local_;

};

class ImportNamespaceSpecifier: public AstNode {
public:
    ImportNamespaceSpecifier();

    Sp<Identifier> local_;

};

class ImportSpecifier: public AstNode {
public:
    ImportSpecifier();

    Sp<Identifier> local_;
    Sp<Identifier> imported_;

};

class LabeledStatement: public AstNode {
public:
    LabeledStatement();

    Sp<Identifier> label_;
    Sp<Statement> body_;

};

class Literal: public Expression {
public:
    Literal();

    std::optional<std::variant<bool, JS_Number, UString>> value_;
    UString raw_;

};

class MetaProperty: public AstNode {
public:
    MetaProperty();

    Sp<Identifier> meta_;
    Sp<Identifier> property_;

};

class MethodDefinition: public AstNode {
public:
    MethodDefinition();

    Sp<Expression> key_;
    bool computed_ = false;
    Sp<AstNode> value_;
    VarKind kind_ = VarKind::Invalid;
    bool static_ = false;

};

class Module: public AstNode {
public:
    Module();

    std::vector<Sp<AstNode>> body_;
    UString sourceType_;

};

class NewExpression: public Expression {
public:
    NewExpression();

    Sp<Expression> callee_;
    std::vector<Sp<AstNode>> arguments_;

};

class ObjectExpression: public Expression {
public:
    ObjectExpression();

    std::vector<Sp<AstNode>> properties_;

};

class ObjectPattern: public AstNode {
public:
    ObjectPattern();

    std::vector<Sp<AstNode>> properties_;

};

class Property: public AstNode {
public:
    Property();

    Sp<AstNode> key_;
    bool computed_ = false;
    Sp<AstNode> value_;
    VarKind kind_;
    bool method_ = false;
    bool shorthand_ = false;

};

class RegexLiteral: public Expression {
public:
    RegexLiteral();

    JS_RegExp value_;
    UString raw_;

};

class RestElement: public AstNode {
public:
    RestElement();

    Sp<AstNode> argument_;

};

class ReturnStatement: public Statement {
public:
    ReturnStatement();

    Sp<Expression> argument_;

};

class Script: public AstNode {
public:
    Script();

    std::vector<Sp<AstNode>> body_;
    UString sourceType_;

};

class SequenceExpression: public Expression {
public:
    SequenceExpression();

    std::vector<Sp<Expression>> expressions_;

};

class SpreadElement: public AstNode {
public:
    SpreadElement();

    Sp<Expression> argument_;

};

class StaticMemberExpression: public Expression {
public:
    StaticMemberExpression();

    bool computed_ = false;
    Sp<Expression> object_;
    Sp<Expression> property_;

};

class Super: public AstNode {
public:
    Super();

};

class SwitchCase: public AstNode {
public:
    SwitchCase();

    Sp<Expression> test_;
    std::vector<Sp<Statement>> consequent_;

};

class SwitchStatement: public Statement {
public:
    SwitchStatement();

    Sp<Expression> discriminant_;
    std::vector<Sp<SwitchCase>> cases_;

};

class TaggedTemplateExpression: public Expression {
public:
    TaggedTemplateExpression();

    Sp<Expression> tag_;
    Sp<TemplateLiteral> quasi_;

};

class TemplateElementValue {
public:
    TemplateElementValue() : raw_(UString()) {}

    UString cooked_;
    UString raw_;

};

class TemplateElement: public AstNode {
public:
    TemplateElement();

    TemplateElementValue value_;
    bool tail_ = false;

};

class TemplateLiteral: public AstNode {
public:
    TemplateLiteral();

    std::vector<Sp<TemplateElement>> quasis_;
    std::vector<Sp<Expression>> expressions_;

};

class ThisExpression: public Expression {
public:
    ThisExpression();

};

class ThrowStatement: public Statement {
public:
    ThrowStatement();

    Sp<Expression> argument_;

};

class TryStatement: public Statement {
public:
    TryStatement();

    Sp<BlockStatement> block_;
    Sp<CatchClause> handler_;
    Sp<BlockStatement> finalizer_;

};

class UnaryExpression: public Expression {
public:
    UnaryExpression();

    UString operator_;
    Sp<Expression> argument_;
    bool prefix_ = false;

};

class UpdateExpression: public Expression {
public:
    UpdateExpression();

    UString operator_;
    Sp<Expression> argument_;
    bool prefix_ = false;

};

class VariableDeclaration: public Declaration {
public:
    VariableDeclaration();

    std::vector<Sp<VariableDeclarator>> declarations_;
    VarKind kind_;

};

class VariableDeclarator: public AstNode {
public:
    VariableDeclarator();

    Sp<AstNode> id_;
    Sp<Expression> init_;

};

class WhileStatement: public Statement {
public:
    WhileStatement();

    Sp<Expression> test_;
    Sp<Statement> body_;

};

class WithStatement: public Statement {
public:
    WithStatement();

    Sp<Expression> object_;
    Sp<Statement> body_;

};

class YieldExpression: public Expression {
public:
    YieldExpression();

    Sp<Expression> argument_;
    bool delegate_ = false;

};
