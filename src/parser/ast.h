//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <vector>
#include <string>
#include <variant>
#include <optional>
#include "../gc.hpp"

typedef double JSNumber;
typedef std::string JSRegExp;

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

class ArgumentListElement;

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

    std::vector<AstNode*> elements_;

};

class ArrayPattern: public AstNode {
public:
    ArrayPattern();

    std::vector<AstNode*> elements_;

};

class ArrowFunctionExpression: public Expression {
public:
    ArrowFunctionExpression();

    Identifier* id_ = nullptr;
    std::vector<AstNode*> params_;
    std::variant<BlockStatement*, Expression*> body_;
    // TODO: flags
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;
};

class AssignmentExpression: public Expression {
public:
    AssignmentExpression();

    std::string operator_;
    Expression* left_ = nullptr;
    Expression* right_ = nullptr;
};

class AssignmentPattern: public AstNode {
public:
    AssignmentPattern();

    AstNode* left_;
    Expression* right_ = nullptr;
};

class AsyncArrowFunctionExpression: public Expression {
public:
    AsyncArrowFunctionExpression();

    Identifier* id_ = nullptr;
    std::vector<AstNode*> params_;
    std::variant<BlockStatement*, Expression*> body_;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;
};

class AsyncFunctionDeclaration: public Declaration {
public:
    AsyncFunctionDeclaration();

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

    Identifier* id_ = nullptr;
    std::vector<AstNode*> params;
    BlockStatement* body_ = nullptr;
    bool generator_ = false;
    bool expression_ = false;
    bool async_ = false;
};

class AwaitExpression: public Expression {
public:
    AwaitExpression();

    Expression* arguments_ = nullptr;
};

class BinaryExpression: public Expression {
public:
    BinaryExpression();

    std::string operator_;
    Expression* left_ = nullptr;
    Expression* right_ = nullptr;
};

class BlockStatement: public AstNode {
public:
    BlockStatement();

    std::vector<Statement*> body_;

};

class BreakStatement: public Statement {
public:
    BreakStatement();

    Identifier* label_ = nullptr;

};

class CallExpression: public Expression {
public:
    CallExpression();

    std::variant<Expression*, Import*> callee_;
    std::vector<ArgumentListElement*> arguments_;

};

class CatchClause: public AstNode {
public:
    CatchClause();

    AstNode* param_;
    BlockStatement* body_ = nullptr;

};

class ClassBody: public AstNode {
public:
    ClassBody();

    std::vector<Property*> body_;

};

class ClassDeclaration: public Declaration {
public:
    ClassDeclaration();

    Identifier* id_ = nullptr;
    Identifier* superClass_ = nullptr;
    ClassBody* body_ = nullptr;

};

class ClassExpression: public Expression {
public:
    ClassExpression();

    Identifier* id_ = nullptr;
    Identifier* superClass_ = nullptr;
    ClassBody* body_ = nullptr;

};

class ComputedMemberExpression: public Expression {
public:
    ComputedMemberExpression();

    bool computed_ = false;
    Expression* object_ = nullptr;
    Expression* property_ = nullptr;

};

class ConditionalExpression: public Expression {
public:
    ConditionalExpression();

    Expression* test_ = nullptr;
    Expression* consequent_ = nullptr;
    Expression* alternate_ = nullptr;

};

class ContinueStatement: public Statement {
public:
    ContinueStatement();

    Identifier* label_ = nullptr;

};

class DebuggerStatement: public Statement {
public:
    DebuggerStatement();

};

class Directive: public Statement {
public:
    Directive();

    Expression* expression_ = nullptr;
    std::string directive_;

};

class DoWhileStatement: public Statement {
public:
    DoWhileStatement();

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

    Literal* source_ = nullptr;

};

class ExportDefaultDeclaration: public Declaration {
public:
    ExportDefaultDeclaration();

    AstNode* declaration_ = nullptr;

};

class ExportNamedDeclaration: public Declaration {
public:
    ExportNamedDeclaration();

    std::vector<AstNode*> declaration_;
    std::vector<ExportSpecifier*> specifiers_;
    Literal* source_ = nullptr;

};

class ExportSpecifier: public AstNode {
public:
    ExportSpecifier();

    Identifier* exported_ = nullptr;
    Identifier* local_ = nullptr;

};

class ExpressionStatement: public Statement {
public:
    ExpressionStatement();

    Expression* expression_ = nullptr;

};

class ForInStatement: public Statement {
public:
    ForInStatement();

    Expression* left_ = nullptr;
    Expression* right_ = nullptr;
    Statement* body_ = nullptr;
    bool each_ = false;

};

class ForOfStatement: public Statement {
public:
    ForOfStatement();

    Expression* left_ = nullptr;
    Expression* right_ = nullptr;
    Statement* body_ = nullptr;

};

class ForStatement: public Statement {
public:
    ForStatement();

    Expression* init_ = nullptr;
    Expression* test_ = nullptr;
    Expression* update_ = nullptr;
    Statement* body_ = nullptr;

};

class FunctionDeclaration: public Declaration {
public:
    FunctionDeclaration();

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

    std::string name_;

};

class IfStatement: public Statement {
public:
    IfStatement();

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

    std::vector<AstNode*> specifiers_;
    Literal* source = nullptr;

};

class ImportDefaultSpecifier: public AstNode {
public:
    ImportDefaultSpecifier();

    Identifier* local_ = nullptr;

};

class ImportNamespaceSpecifier: public AstNode {
public:
    ImportNamespaceSpecifier();

    Identifier* local_ = nullptr;

};

class ImportSpecifier: public AstNode {
public:
    ImportSpecifier();

    Identifier* local_ = nullptr;
    Identifier* imported_ = nullptr;

};

class LabeledStatement: AstNode {
public:
    LabeledStatement();

    Identifier* label_ = nullptr;
    Statement* body_ = nullptr;

};

class Literal: Expression {
public:
    Literal();

    std::optional<std::variant<bool, JSNumber, std::string>> value_;
    std::string raw_;

};

class MetaProperty: AstNode {
public:
    MetaProperty();

    Identifier* meta_ = nullptr;
    Identifier* property_ = nullptr;

};

class MethodDefinition: public AstNode {
public:
    MethodDefinition();

    Expression* key_ = nullptr;
    bool computed_ = false;
    std::optional<std::variant<AsyncFunctionExpression*, FunctionExpression*>> value_;
    std::string kind_;
    bool static_ = false;

};

class Module: public AstNode {
public:
    Module();

    std::vector<AstNode*> body_;
    std::string sourceType_;

};

class NewExpression: public Expression {
public:
    NewExpression();

    Expression* callee_ = nullptr;
    std::vector<ArgumentListElement*> arguments_;

};

class ObjectExpression: public Expression {
public:
    ObjectExpression();

    std::vector<AstNode*> properties_;

};

class ObjectPattern: public AstNode {
public:
    ObjectPattern();

    std::vector<AstNode*> properties_;

};

class Property: public AstNode {
public:
    Property();

    AstNode* key_ = nullptr;
    bool computed_;
    AstNode* value_ = nullptr;
    std::string kind_;
    bool method_;
    bool shorthand_;

};

class RegexLiteral: public Expression {
public:
    RegexLiteral();

    JSRegExp value_;
    std::string raw_;

};

class RestElement: public AstNode {
public:
    RestElement();

    std::variant<BindingIdentifier*, AstNode*> argument_;

};

class ReturnStatement: public Statement {
public:
    ReturnStatement();

    Expression* argument_ = nullptr;

};

class Script: public AstNode {
public:
    Script();

    std::vector<AstNode*> body_;
    std::string sourceType_;

};

class SequenceExpression: public Expression {
public:
    SequenceExpression();

    std::vector<Expression*> expressions_;

};

class SpreadElement: public AstNode {
public:
    SpreadElement();

    Expression* argument_ = nullptr;

};

class StaticMemberExpression: public Expression {
public:
    StaticMemberExpression();

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

    Expression *test = nullptr;
    std::vector<Statement *> consequent_;

};

class SwitchStatement: public Statement {
public:
    SwitchStatement();

    Expression* discriminant_ = nullptr;
    std::vector<SwitchCase*> cases_;

};

class TaggedTemplateExpression: public Expression {
public:
    TaggedTemplateExpression();

    Expression* tag_ = nullptr;
    TemplateLiteral* quasi_ = nullptr;

};

class TemplateElementValue {
public:
    std::string cooked_;
    std::string raw_;

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

    Expression* argument_ = nullptr;

};

class TryStatement: public Statement {
public:
    TryStatement();

    BlockStatement* block_ = nullptr;
    CatchClause* handler_ = nullptr;
    BlockStatement* finalizer_ = nullptr;

};

class UnaryExpression: public Expression {
public:
    UnaryExpression();

    std::string operator_;
    Expression* argument_ = nullptr;
    bool prefix_ = false;

};

class UpdateExpression: public Expression {
public:
    UpdateExpression();

    std::string operator_;
    Expression* argument_ = nullptr;
    bool prefix_ = false;

};

class VariableDeclaration: public Declaration {
public:
    VariableDeclaration();

    std::vector<VariableDeclarator*> declarations_;
    std::string kind_;

};

class VariableDeclarator: public AstNode {
public:
    VariableDeclarator();

    std::variant<BindingIdentifier*, AstNode*> id_;
    Expression* init_ = nullptr;

};

class WhileStatement: public Statement {
public:
    WhileStatement();

    Expression* test_ = nullptr;
    Statement* body_ = nullptr;

};

class WithStatement: public Statement {
public:
    WithStatement();

    Expression* object_ = nullptr;
    Statement* body_ = nullptr;

};

class YieldExpression: public Expression {
public:
    YieldExpression();

    Expression* argument_ = nullptr;
    bool delegate_ = false;

};
