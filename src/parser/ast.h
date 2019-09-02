//
// Created by Duzhong Chen on 2019/9/3.
//

#ifndef BESTPACK_AST_H
#define BESTPACK_AST_H

#include <vector>
#include <string>
#include <variant>
#include <optional>

typedef double JSNumber;
typedef std::string JSRegExp;

class ArrayExpression;
class ArrayPattern;
class ArrayPatternElement;
class AssignmentPattern;
class AsyncFunctionExpression;
class BindingPattern;
class BlockStatement;
class ClassBody;
class Expression;
class ExportableNamedDeclaration;
class ExportSpecifier;
class ExportAllDeclaration;
class ExportDefaultDeclaration;
class ExportNamedDeclaration;
class FunctionExpression;
class Identifier;
class Import;
class ImportSpecifier;
class ImportDefaultSpecifier;
class ImportNamespaceSpecifier;
class Property;
class RestElement;
class Statement;
class StatementListItem;
class SpreadElement;
class TemplateLiteral;
class VariableDeclarator;

class Literal;
class ExportableDefaultDeclaration;

typedef Identifier BindingIdentifier;
typedef std::variant<Expression*, SpreadElement*> ArgumentListElement;
typedef std::variant<Expression*, SpreadElement*, void> ArrayExpressionElement;
typedef std::variant<Identifier*, Literal*> PropertyKey;
typedef std::variant<
    AssignmentPattern*,
    AsyncFunctionExpression*,
    BindingIdentifier*,
    BindingPattern*,
    FunctionExpression*
> PropertyValue;
typedef std::variant<Property*, RestElement*> ObjectPatternProperty;
typedef std::variant<Property*, SpreadElement*> ObjectExpressionProperty;
typedef std::variant<ImportDefaultSpecifier*, ImportNamespaceSpecifier*, ImportSpecifier*> ImportDeclarationSpecifier;
typedef std::variant<AssignmentPattern*, BindingIdentifier*, BindingPattern> FunctionParameter;

class AstNode {
public:

    virtual bool IsDeclaration() const { return false; }
    virtual bool IsExpression() const { return false; }
    virtual bool IsStatement() const { return false; }

    ~AstNode() = delete;
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
    std::vector<ArrayExpressionElement> elements_;

};

class ArrayPattern: public AstNode {
public:
    std::vector<ArrayPatternElement> elements_;

};

class ArrowFunctionExpression: public Expression {
public:
    Identifier* id_;
    std::vector<FunctionParameter> params_;
    std::variant<BlockStatement*, Expression*> body_;
    // TODO: flags
    bool generator_;
    bool expression_;
    bool async_;
};

class AssignmentExpression: public Expression {
public:
    std::string operator_;
    Expression* left_;
    Expression* right_;
};

class AssignmentPattern: public AstNode {
public:
    std::variant<BindingIdentifier*, BindingPattern*> left_;
    Expression* right_;
};

class AsyncArrowFunctionExpression: public Expression {
public:
    Identifier* id_;
    std::vector<FunctionParameter*> params_;
    std::variant<BlockStatement*, Expression*> body_;
    bool generator_;
    bool expression_;
    bool async_;
};

class AsyncFunctionDeclaration: public Declaration {
public:
    Identifier* id_;
    std::vector<FunctionParameter*> params_;
    BlockStatement* body_;
    bool generator_;
    bool expression_;
    bool async_;
};

class AsyncFunctionExpression: public Expression {
public:
    Identifier* id_;
    std::vector<FunctionParameter> params;
    BlockStatement* body_;
    bool generator_;
    bool expression_;
    bool async_;
};

class AwaitExpression: public Expression {
public:
    Expression* arguments_;
};

class BinaryExpression: public Expression {
public:
    std::string operator_;
    Expression* left_;
    Expression* right_;
};

class BlockStatement: public AstNode {
public:
    std::vector<Statement*> body_;
};

class BreakStatement: public Statement {
public:
    Identifier* label_;
};

class CallExpression: public Expression {
public:
    std::variant<Expression*, Import*> callee_;
    std::vector<ArgumentListElement*> arguments_;
};

class CatchClause: public AstNode {
public:
    std::variant<BindingIdentifier*, BindingPattern*> param_;
    BlockStatement* body_;
};

class ClassDeclaration: public Declaration {
public:
    Identifier* id_;
    Identifier* superClass_;
    ClassBody* body_;
};

class ClassExpression: public Expression {
public:
    Identifier* id_;
    Identifier* superClass_;
    ClassBody* body_;
};

class ComputedMemberExpression: public Expression {
public:
    bool computed_;
    Expression* object_;
    Expression* property_;
};

class ConditionalExpression: public Expression {
public:
    Expression* test_;
    Expression* consequent_;
    Expression* alternate_;
};

class ContinueStatement: public Statement {
public:
    Identifier* label_;
};

class DebuggerStatement: public Statement {

};

class Directive: public Statement {
public:
    Expression* expression_;
    std::string directive_;
};

class DoWhileStatement: public Statement {
public:
    Statement* body_;
    Expression* test_;
};

class EmptyStatement: public Statement {

};

class ExportAllDeclaration: public Declaration {
public:
    Literal* source_;
};

class ExportDefaultDeclaration: public Declaration {
public:
    ExportableDefaultDeclaration* declaration_;
};

class ExportNamedDeclaration: public Declaration {
public:
    std::vector<ExportableNamedDeclaration*> declaration_;
    std::vector<ExportSpecifier*> specifiers_;
    Literal* source_;
};

class ExportSpecifier: public AstNode {
public:
    Identifier* exported_;
    Identifier* local_;
};

class ExpressionStatement: public Statement {
public:
    Expression* expression_;
};

class ForInStatement: public Statement {
public:
    Expression* left_;
    Expression* right_;
    Statement* body_;
    bool each;
};

class ForOfStatement: public Statement {
public:
    Expression* left_;
    Expression* right_;
    Statement* body_;
};

class ForStatement: public Statement {
public:
    Expression* init_;
    Expression* test_;
    Expression* update_;
    Statement* body_;
};

class FunctionDeclaration: public Declaration {
public:
    Identifier* id_;
    std::vector<FunctionParameter*> params_;
    BlockStatement* body_;
    bool generator_;
    bool expression_;
    bool async_;
};

class FunctionExpression: public Expression {
public:
    Identifier* id_;
    std::vector<FunctionParameter*> params_;
    BlockStatement* body_;
    bool generator_;
    bool expression_;
    bool async_;
};

class Identifier: public Expression {
public:
    std::string name_;
};

class IfStatement: public Statement {
public:
    Expression* test_;
    Statement* consequent_;
    Statement* alternate_;
};

class Import: public AstNode {

};

class ImportDeclaration: public Declaration {
public:
    std::vector<ImportDeclarationSpecifier> specifiers_;
    Literal* source;
};

class ImportDefaultSpecifier: public AstNode {
public:
    Identifier* local_;
};

class ImportNamespaceSpecifier: public AstNode {
public:
    Identifier* local_;
};

class ImportSpecifier: public AstNode {
public:
    Identifier* local_;
    Identifier* imported_;

};

class LabeledStatement: AstNode {
public:
    Identifier* label_;
    Statement* body_;

};

class Literal: Expression {
public:
    std::optional<std::variant<bool, JSNumber, std::string>> value_;
    std::string raw_;
};

class MetaProperty: AstNode {
public:
    Identifier* meta_;
    Identifier* property_;
};

class MethodDefinition: public AstNode {
public:
    Expression* key_;
    bool computed_;
    std::optional<std::variant<AsyncFunctionExpression*, FunctionExpression*>> value_;
    std::string kind_;
    bool static_;
};

class Module: public AstNode {
public:
    std::vector<StatementListItem*> body_;
    std::string sourceType_;

};

class NewExpression: public Expression {
public:
    Expression* callee_;
    std::vector<ArgumentListElement*> arguments_;
};

class ObjectExpression: public Expression {
public:
    std::vector<ObjectExpressionProperty> properties_;
};

class ObjectPattern: public AstNode {
public:
    std::vector<ObjectPatternProperty> properties_;
};

class Property: public AstNode {
public:
    PropertyKey key_;
    bool computed_;
    PropertyValue value;
    std::string kind_;
    bool method_;
    bool shorthand_;
};

class RegexLiteral: public Expression {
public:
    JSRegExp value_;
    std::string raw_;
};

class RestElement: public AstNode {
public:
    std::variant<BindingIdentifier*, BindingPattern*> argument_;
};

class ReturnStatement: public Statement {
public:
    Expression* argument_;
};

class Script: public AstNode {
public:
    std::vector<StatementListItem*> body_;
    std::string sourceType_;

};

class SequenceExpression: public Expression {
public:
    std::vector<Expression*> expressions_;
};

class SpreadElement: public AstNode {
public:
    Expression* argument_;
};

class StaticMemberExpression: public Expression {
public:
    bool computed_;
    Expression* object_;
    Expression* property_;
};

class Super: public AstNode {

};

class SwitchCase: public AstNode {
public:
    Expression *test;
    std::vector<Statement *> consequent_;
};

class SwitchStatement: public Statement {
public:
    Expression* discriminant_;
    std::vector<SwitchCase*> cases_;
};

class TaggedTemplateExpression: public Expression {
public:
    Expression* tag_;
    TemplateLiteral* quasi_;
};

class TemplateElementValue {
public:
    std::string cooked_;
    std::string raw_;
};

class TemplateElement: public AstNode {
public:
    TemplateElementValue value_;
    bool tail_;
};

class TemplateLiteral: public AstNode {
public:
    std::vector<TemplateElement*> quasis_;
    std::vector<Expression*> expressions_;
};

class ThisExpression: public Expression {

};

class ThrowStatement: public Statement {
public:
    Expression* argument_;

};

class TryStatement: public Statement {
public:
    BlockStatement* block_;
    CatchClause* handler_;
    BlockStatement* finalizer_;
};

class UnaryExpression: public Expression {
public:
    std::string operator_;
    Expression* argument_;
    bool prefix_;
};

class UpdateExpression: public Expression {
public:
    std::string operator_;
    Expression* argument_;
    bool prefix_;
};

class VariableDeclaration: public Declaration {
public:
    std::vector<VariableDeclarator*> declarations_;
    std::string kind_;
};

class VariableDeclarator: public AstNode {
public:
    std::variant<BindingIdentifier*, BindingPattern*> id_;
    Expression* init_;
};

class WhileStatement: public Statement {
public:
    Expression* test_;
    Statement* body_;
};

class WithStatement: public Statement {
public:
    Expression* object_;
    Statement* body_;
};

class YieldExpression: public Expression {
public:
    Expression* argument_;
    bool delegate_;
};

#endif //BESTPACK_AST_H
