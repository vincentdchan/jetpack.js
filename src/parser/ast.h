//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <vector>
#include <string>
#include <variant>
#include <optional>
#include "../tokenizer/token.h"
#include "../gc.hpp"
#include "../js_string.h"

typedef double JS_Number;
typedef JS_String JS_RegExp;

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

static const char* AstNodeTypeToCString(AstNodeType t);

class AstNode: public GarbageCollector::ObjectHeader {
public:

    AstNodeType type_;

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
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<AstNode*> elements_;

};

class ArrayPattern: public AstNode {
public:
    ArrayPattern();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<AstNode*> elements_;

};

class ArrowFunctionExpression: public Expression {
public:
    ArrowFunctionExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* id_ = nullptr;
    std::vector<AstNode*> params_;
    AstNode* body_;
    // TODO: flags
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

class AssignmentExpression: public Expression {
public:
    AssignmentExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    JS_String operator_;
    Expression* left_ = nullptr;
    Expression* right_ = nullptr;

};

class AssignmentPattern: public AstNode {
public:
    AssignmentPattern();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    AstNode* left_ = nullptr;
    Expression* right_ = nullptr;
};

class AsyncArrowFunctionExpression: public Expression {
public:
    AsyncArrowFunctionExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* id_ = nullptr;
    std::vector<AstNode*> params_;
    AstNode* body_;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;
};

class AsyncFunctionDeclaration: public Declaration {
public:
    AsyncFunctionDeclaration();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* id_ = nullptr;
    std::vector<AstNode*> params_;
    BlockStatement* body_ = nullptr;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;
};

class AsyncFunctionExpression: public Expression {
public:
    AsyncFunctionExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* id_ = nullptr;
    std::vector<AstNode*> params_;
    BlockStatement* body_ = nullptr;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;
};

class AwaitExpression: public Expression {
public:
    AwaitExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* arguments_ = nullptr;
};

class BinaryExpression: public Expression {
public:
    BinaryExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    JS_String operator_;
    Expression* left_ = nullptr;
    Expression* right_ = nullptr;
};

class BlockStatement: public AstNode {
public:
    BlockStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<Statement*> body_;

};

class BreakStatement: public Statement {
public:
    BreakStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* label_ = nullptr;

};

class CallExpression: public Expression {
public:
    CallExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    AstNode* callee_ = nullptr;
    std::vector<AstNode*> arguments_;

};

class CatchClause: public AstNode {
public:
    CatchClause();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    AstNode* param_ = nullptr;
    BlockStatement* body_ = nullptr;

};

class ClassBody: public AstNode {
public:
    ClassBody();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<Property*> body_;

};

class ClassDeclaration: public Declaration {
public:
    ClassDeclaration();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* id_ = nullptr;
    Identifier* superClass_ = nullptr;
    ClassBody* body_ = nullptr;

};

class ClassExpression: public Expression {
public:
    ClassExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* id_ = nullptr;
    Identifier* superClass_ = nullptr;
    ClassBody* body_ = nullptr;

};

class ComputedMemberExpression: public Expression {
public:
    ComputedMemberExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    bool computed_ = false;
    Expression* object_ = nullptr;
    Expression* property_ = nullptr;

};

class ConditionalExpression: public Expression {
public:
    ConditionalExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* test_ = nullptr;
    Expression* consequent_ = nullptr;
    Expression* alternate_ = nullptr;

};

class ContinueStatement: public Statement {
public:
    ContinueStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* label_ = nullptr;

};

class DebuggerStatement: public Statement {
public:
    DebuggerStatement();

};

class Directive: public Statement {
public:
    Directive();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* expression_ = nullptr;
    JS_String directive_;

};

class DoWhileStatement: public Statement {
public:
    DoWhileStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Statement* body_ = nullptr;
    Expression* test_ = nullptr;

};

class EmptyStatement: public Statement {
public:
    EmptyStatement();

};

class ExportAllDeclaration: public Declaration {
public:
    ExportAllDeclaration();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Literal* source_ = nullptr;

};

class ExportDefaultDeclaration: public Declaration {
public:
    ExportDefaultDeclaration();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    AstNode* declaration_ = nullptr;

};

class ExportNamedDeclaration: public Declaration {
public:
    ExportNamedDeclaration();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<AstNode*> declaration_;
    std::vector<ExportSpecifier*> specifiers_;
    Literal* source_ = nullptr;

};

class ExportSpecifier: public AstNode {
public:
    ExportSpecifier();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* exported_ = nullptr;
    Identifier* local_ = nullptr;

};

class ExpressionStatement: public Statement {
public:
    ExpressionStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* expression_ = nullptr;

};

class ForInStatement: public Statement {
public:
    ForInStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* left_ = nullptr;
    Expression* right_ = nullptr;
    Statement* body_ = nullptr;
    bool each_ = false;

};

class ForOfStatement: public Statement {
public:
    ForOfStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* left_ = nullptr;
    Expression* right_ = nullptr;
    Statement* body_ = nullptr;

};

class ForStatement: public Statement {
public:
    ForStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* init_ = nullptr;
    Expression* test_ = nullptr;
    Expression* update_ = nullptr;
    Statement* body_ = nullptr;

};

class FunctionDeclaration: public Declaration {
public:
    FunctionDeclaration();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* id_ = nullptr;
    std::vector<AstNode*> params_;
    BlockStatement* body_ = nullptr;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

class FunctionExpression: public Expression {
public:
    FunctionExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* id_ = nullptr;
    std::vector<AstNode*> params_;
    BlockStatement* body_ = nullptr;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;

};

class Identifier: public Expression {
public:
    Identifier();

    JS_String name_;

};

class IfStatement: public Statement {
public:
    IfStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* test_ = nullptr;
    Statement* consequent_ = nullptr;
    Statement* alternate_ = nullptr;

};

class Import: public AstNode {
public:
    Import();

};

class ImportDeclaration: public Declaration {
public:
    ImportDeclaration();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<AstNode*> specifiers_;
    Literal* source_ = nullptr;

};

class ImportDefaultSpecifier: public AstNode {
public:
    ImportDefaultSpecifier();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* local_ = nullptr;

};

class ImportNamespaceSpecifier: public AstNode {
public:
    ImportNamespaceSpecifier();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* local_ = nullptr;

};

class ImportSpecifier: public AstNode {
public:
    ImportSpecifier();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* local_ = nullptr;
    Identifier* imported_ = nullptr;

};

class LabeledStatement: public AstNode {
public:
    LabeledStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* label_ = nullptr;
    Statement* body_ = nullptr;

};

class Literal: public Expression {
public:
    Literal();

    std::optional<std::variant<bool, JS_Number, JS_String>> value_;
    JS_String raw_;

};

class MetaProperty: public AstNode {
public:
    MetaProperty();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Identifier* meta_ = nullptr;
    Identifier* property_ = nullptr;

};

class MethodDefinition: public AstNode {
public:
    MethodDefinition();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* key_ = nullptr;
    bool computed_ = false;
    AstNode* value_ = nullptr;
    JS_String kind_;
    bool static_ = false;

};

class Module: public AstNode {
public:
    Module();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<AstNode*> body_;
    JS_String sourceType_;

};

class NewExpression: public Expression {
public:
    NewExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* callee_ = nullptr;
    std::vector<AstNode*> arguments_;

};

class ObjectExpression: public Expression {
public:
    ObjectExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<AstNode*> properties_;

};

class ObjectPattern: public AstNode {
public:
    ObjectPattern();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<AstNode*> properties_;

};

class Property: public AstNode {
public:
    Property();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    AstNode* key_ = nullptr;
    bool computed_ = false;
    AstNode* value_ = nullptr;
    JS_String kind_;
    bool method_ = false;
    bool shorthand_ = false;

};

class RegexLiteral: public Expression {
public:
    RegexLiteral();

    JS_RegExp value_;
    JS_String raw_;

};

class RestElement: public AstNode {
public:
    RestElement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    AstNode* argument_ = nullptr;

};

class ReturnStatement: public Statement {
public:
    ReturnStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* argument_ = nullptr;

};

class Script: public AstNode {
public:
    Script();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<AstNode*> body_;
    JS_String sourceType_;

};

class SequenceExpression: public Expression {
public:
    SequenceExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<Expression*> expressions_;

};

class SpreadElement: public AstNode {
public:
    SpreadElement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* argument_ = nullptr;

};

class StaticMemberExpression: public Expression {
public:
    StaticMemberExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    bool computed_ = false;
    Expression* object_ = nullptr;
    Expression* property_ = nullptr;

};

class Super: public AstNode {
public:
    Super();

};

class SwitchCase: public AstNode {
public:
    SwitchCase();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression *test_ = nullptr;
    std::vector<Statement *> consequent_;

};

class SwitchStatement: public Statement {
public:
    SwitchStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* discriminant_ = nullptr;
    std::vector<SwitchCase*> cases_;

};

class TaggedTemplateExpression: public Expression {
public:
    TaggedTemplateExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* tag_ = nullptr;
    TemplateLiteral* quasi_ = nullptr;

};

class TemplateElementValue {
public:
    JS_String cooked_;
    JS_String raw_;

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
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<TemplateElement*> quasis_;
    std::vector<Expression*> expressions_;

};

class ThisExpression: public Expression {
public:
    ThisExpression();

};

class ThrowStatement: public Statement {
public:
    ThrowStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* argument_ = nullptr;

};

class TryStatement: public Statement {
public:
    TryStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    BlockStatement* block_ = nullptr;
    CatchClause* handler_ = nullptr;
    BlockStatement* finalizer_ = nullptr;

};

class UnaryExpression: public Expression {
public:
    UnaryExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    JS_String operator_;
    Expression* argument_ = nullptr;
    bool prefix_ = false;

};

class UpdateExpression: public Expression {
public:
    UpdateExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    JS_String operator_;
    Expression* argument_ = nullptr;
    bool prefix_ = false;

};

class VariableDeclaration: public Declaration {
public:
    VariableDeclaration();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    std::vector<VariableDeclarator*> declarations_;
    JS_String kind_;

};

class VariableDeclarator: public AstNode {
public:
    VariableDeclarator();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    AstNode* id_ = nullptr;
    Expression* init_ = nullptr;

};

class WhileStatement: public Statement {
public:
    WhileStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* test_ = nullptr;
    Statement* body_ = nullptr;

};

class WithStatement: public Statement {
public:
    WithStatement();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* object_ = nullptr;
    Statement* body_ = nullptr;

};

class YieldExpression: public Expression {
public:
    YieldExpression();
    void MarkChildren(GarbageCollector::MarkFunction marker) override;

    Expression* argument_ = nullptr;
    bool delegate_ = false;

};
