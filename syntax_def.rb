
class Property

  attr_accessor :prop_type, :name

  def initialize(type, name)
    @prop_type = type
    @name = name
  end

end

class SyntaxDefinition

  attr_accessor :class_id, :base, :is_virtual, :props

  def initialize(class_id, base, is_virtual)
    @class_id = class_id
    @base = base
    @is_virtual = is_virtual
    @props = []
  end

  def def_prop(type_name, name)
    prop = Property.new type_name ,name
    @props.append prop
  end

end

class SyntaxFactory

  @@syntaxes = []

  def self.syntaxes
    @@syntaxes
  end

  def self.def_syntax(class_id, base: nil, is_virtual: false, &block)
    node = SyntaxDefinition.new class_id, base, is_virtual
    node.instance_eval(&block)
    @@syntaxes.append(node)
  end

end

class Option

  attr_accessor :value

  def initialize(value)
    @value = value
  end

end

class Variant

  attr_accessor :elements

  def initialize(elements)
    @elements = elements
  end

end

class Symbol

  def opt
    Option.new self
  end

end

class Array

  def variant
    Variant.new self
  end

end


SyntaxFactory.def_syntax :SyntaxNode, is_virtual: true do
  # empty
end

SyntaxFactory.def_syntax :Expression, base: :SyntaxNode, is_virtual: true do
end

SyntaxFactory.def_syntax :Statement, base: :SyntaxNode, is_virtual: true do
end

SyntaxFactory.def_syntax :Pattern, base: :SyntaxNode, is_virtual: true do
end

SyntaxFactory.def_syntax :Declaration, base: :Statement, is_virtual: true do
end

SyntaxFactory.def_syntax :ArrayExpression, base: :Expression do
  def_prop [:SyntaxNode.opt], "elements"
end

SyntaxFactory.def_syntax :ArrayPattern, base: :Pattern do
  def_prop [:SyntaxNode.opt], "elements"
end

SyntaxFactory.def_syntax :ArrowFunctionExpression, base: :Expression do
  def_prop :Identifier.opt, "id"
  def_prop [:SyntaxNode], "params"
  def_prop :SyntaxNode, "body"
  def_prop :Boolean, "generator"
  def_prop :Boolean, "expression"
  def_prop :Boolean, "async"
end

SyntaxFactory.def_syntax :AssignmentExpression, base: :Expression do
  def_prop :String, "operator"
  def_prop :Pattern, "left"
  def_prop :Expression, "right"
end

SyntaxFactory.def_syntax :AssignmentPattern, base: :Pattern do
  def_prop :SyntaxNode, "left"
  def_prop :Expression, "right"
end

SyntaxFactory.def_syntax :AwaitExpression, base: :Expression do
  def_prop :Expression, "argument"
end

SyntaxFactory.def_syntax :BinaryExpression, base: :Expression do
  def_prop :String, "operator"
  def_prop :Expression, "left"
  def_prop :Expression, "right"
end

SyntaxFactory.def_syntax :BlockStatement, base: :Statement do
  def_prop [:SyntaxNode], "body"
end

SyntaxFactory.def_syntax :BreakStatement, base: :Statement do
  def_prop :Identifier.opt, "label"
end

SyntaxFactory.def_syntax :CallExpression, base: :Expression do
  def_prop :Expression, "callee"
  def_prop [:SyntaxNode], "arguments"
end

SyntaxFactory.def_syntax :CatchClause, base: :SyntaxNode do
  def_prop :SyntaxNode, "param"
  def_prop :BlockStatement, "body"
end

SyntaxFactory.def_syntax :ClassBody, base: :SyntaxNode do
  def_prop [:MethodDefinition], "body"
end

SyntaxFactory.def_syntax :ClassDeclaration, base: :Declaration do
  def_prop :Identifier.opt, "id"
  def_prop :Identifier.opt, "superClass"
  def_prop :ClassBody, "body"
end

SyntaxFactory.def_syntax :ClassExpression, base: :Expression do
  def_prop :Identifier.opt, "id"
  def_prop :Identifier.opt, "superClass"
  def_prop :ClassBody.opt, "body"
end

SyntaxFactory.def_syntax :ConditionalExpression, base: :Expression do
  def_prop :Expression, "test"
  def_prop :Expression, "consequent"
  def_prop :Expression, "alternate"
end

SyntaxFactory.def_syntax :ContinueStatement, base: :Statement do
  def_prop :Identifier.opt, "label"
end

SyntaxFactory.def_syntax :DebuggerStatement, base: :Statement do
end

SyntaxFactory.def_syntax :Directive, base: :Statement do
  def_prop :Expression, "expression"
  def_prop :String, "directive"
end

SyntaxFactory.def_syntax :DoWhileStatement, base: :Statement do
  def_prop :Statement, "body"
  def_prop :Expression, "test"
end

SyntaxFactory.def_syntax :EmptyStatement, base: :Statement do
end

SyntaxFactory.def_syntax :ExportAllDeclaration, base: :Declaration do
  def_prop :Literal, "source"
end

SyntaxFactory.def_syntax :ExportDefaultDeclaration, base: :Declaration do
  def_prop :SyntaxNode, "declaration"
end

SyntaxFactory.def_syntax :ExportNamedDeclaration, base: :Declaration do
  def_prop :SyntaxNode.opt, "declaration"
  def_prop [:ExportSpecifier], "specifiers"
  def_prop :Literal.opt, "source"
end

SyntaxFactory.def_syntax :ExportSpecifier, base: :SyntaxNode do
  def_prop :Identifier, "exported"
  def_prop :Identifier, "local"
end

SyntaxFactory.def_syntax :ExpressionStatement, base: :Statement do
  def_prop :Expression, "expression"
end

SyntaxFactory.def_syntax :ForInStatement, base: :Statement do
  def_prop :SyntaxNode, "left"
  def_prop :SyntaxNode, "right"
  def_prop :Statement, "body"
  def_prop :Boolean, "each"
end

SyntaxFactory.def_syntax :ForOfStatement, base: :Statement do
  def_prop :SyntaxNode, "left"
  def_prop :SyntaxNode, "right"
  def_prop :Statement, "body"
end

SyntaxFactory.def_syntax :ForStatement, base: :Statement do
  def_prop :SyntaxNode.opt, "init"
  def_prop :SyntaxNode.opt, "test"
  def_prop :SyntaxNode.opt, "update"
  def_prop :Statement, "body"
end

SyntaxFactory.def_syntax :FunctionDeclaration, base: :Declaration do
  def_prop :Identifier.opt, "id"
  def_prop [:SyntaxNode], "params"
  def_prop :BlockStatement, "body"
  def_prop :Boolean, "generator"
  def_prop :Boolean, "expression"
  def_prop :Boolean, "async"
end

SyntaxFactory.def_syntax :FunctionExpression, base: :Expression do
  def_prop :Identifier.opt, "id"
  def_prop [:SyntaxNode], "params"
  def_prop :BlockStatement, "body"
  def_prop :Boolean, "generator"
  def_prop :Boolean, "expression"
  def_prop :Boolean, "async"
end

SyntaxFactory.def_syntax :Identifier, base: [:Expression, :Pattern] do
  def_prop :String, "name"
end

SyntaxFactory.def_syntax :IfStatement, base: :Statement do
  def_prop :Expression, "test"
  def_prop :Statement, "consequent"
  def_prop :Statement.opt, "alternate"
end

SyntaxFactory.def_syntax :Import, base: :Expression do
end

SyntaxFactory.def_syntax :ImportDeclaration, base: :Declaration do
  def_prop [:SyntaxNode], "specifiers"
  def_prop :Literal, "source"
end

SyntaxFactory.def_syntax :ImportDefaultSpecifier, base: :SyntaxNode do
  def_prop :Identifier, "local"
end

SyntaxFactory.def_syntax :ImportNamespaceSpecifier, base: :SyntaxNode do
  def_prop :Identifier, "local"
end

SyntaxFactory.def_syntax :ImportSpecifier, base: :SyntaxNode do
  def_prop :Identifier, "local"
  def_prop :Identifier, "imported"
end

SyntaxFactory.def_syntax :LabeledStatement, base: :Statement do
  def_prop :Identifier, "label"
  def_prop :Statement, "body"
end

SyntaxFactory.def_syntax :Literal, base: :Expression do
  def_prop [:Boolean, :Number, :String].variant, "value"
  def_prop :String, "raw"
end

SyntaxFactory.def_syntax :MetaProperty, base: :Expression do
  def_prop :Identifier, "meta"
  def_prop :Identifier, "property"
end

SyntaxFactory.def_syntax :MethodDefinition, base: :SyntaxNode do
  def_prop :SyntaxNode.opt, "key"
  def_prop :Boolean, "computed"
  def_prop :Expression.opt, "value"
  def_prop :VarKind, "kind"
  def_prop :Boolean, "static"
end

SyntaxFactory.def_syntax :Module, base: :SyntaxNode do
  def_prop [:SyntaxNode], "body"
  def_prop :String, "sourceType"
  def_prop [:Comment], "comments"
end

SyntaxFactory.def_syntax :NewExpression, base: :Expression do
  def_prop :Expression, "callee"
  def_prop [:SyntaxNode], "arguments"
end

SyntaxFactory.def_syntax :ObjectExpression, base: :Expression do
  def_prop [:SyntaxNode], "properties"
end

SyntaxFactory.def_syntax :ObjectPattern, base: :Pattern do
  def_prop [:SyntaxNode], "properties"
end

SyntaxFactory.def_syntax :Property, base: :SyntaxNode do
  def_prop :SyntaxNode, "key"
  def_prop :Boolean, "computed"
  def_prop :SyntaxNode.opt, "value"
  def_prop :VarKind, "kind"
  def_prop :Boolean, "method"
  def_prop :Boolean, "shorthand"
end

SyntaxFactory.def_syntax :RegexLiteral, base: :Expression do
  def_prop :String, "value"
  def_prop :String, "raw"
end

SyntaxFactory.def_syntax :RestElement, base: [:Pattern, :Expression] do
  def_prop :SyntaxNode, "argument"
end

SyntaxFactory.def_syntax :ReturnStatement, base: :Statement do
  def_prop :Expression.opt, "argument"
end

SyntaxFactory.def_syntax :Script, base: :SyntaxNode do
  def_prop [:SyntaxNode], "body"
  def_prop :String, "sourceType"
  def_prop [:Comment], "comments"
end

SyntaxFactory.def_syntax :SequenceExpression, base: :Expression do
  def_prop [:Expression], "expressions"
end

SyntaxFactory.def_syntax :SpreadElement, base: :SyntaxNode do
  def_prop :Expression, "argument"
end

SyntaxFactory.def_syntax :MemberExpression, base: [:Expression, :Pattern] do
  def_prop :Boolean, "computed"
  def_prop :Expression, "object"
  def_prop :Expression, "property"
end

SyntaxFactory.def_syntax :Super, base: :Expression do
end

SyntaxFactory.def_syntax :SwitchCase, base: :SyntaxNode do
  def_prop :Expression.opt, "test"
  def_prop [:Statement], "consequent"
end

SyntaxFactory.def_syntax :SwitchStatement, base: :Statement do
  def_prop :Expression, "discrimiant"
  def_prop [:SwitchCase], "cases"
end

SyntaxFactory.def_syntax :TaggedTemplateExpression, base: :Expression do
  def_prop :Expression, "tag"
  def_prop :TemplateLiteral, "quasi"
end

SyntaxFactory.def_syntax :TemplateElement, base: :SyntaxNode do
  def_prop :String, "cooked"
  def_prop :String, "raw"
  def_prop :Boolean, "tail"
end

SyntaxFactory.def_syntax :TemplateLiteral, base: :Expression do
  def_prop [:TemplateElement], "quasis"
  def_prop [:Expression], "expressions"
end

SyntaxFactory.def_syntax :ThisExpression, base: :Expression do
end

SyntaxFactory.def_syntax :ThrowStatement, base: :Statement do
  def_prop :Expression, "argument"
end

SyntaxFactory.def_syntax :TryStatement, base: :Statement do
  def_prop :BlockStatement, "block"
  def_prop :CatchClause.opt, "handler"
  def_prop :BlockStatement.opt, "finalizer"
end

SyntaxFactory.def_syntax :UnaryExpression, base: :Expression do
  def_prop :String, "operator"
  def_prop :Expression, "argument"
  def_prop :Boolean, "prefix"
end

SyntaxFactory.def_syntax :UpdateExpression, base: :Expression do
  def_prop :String, "operator"
  def_prop :Expression, "argument"
  def_prop :Boolean, "prefix"
end

SyntaxFactory.def_syntax :VariableDeclaration, base: :Declaration do
  def_prop [:VariableDeclarator], "declarations"
  def_prop :VarKind, "kind"
end

SyntaxFactory.def_syntax :VariableDeclarator, base: :SyntaxNode do
  def_prop :SyntaxNode, "id"
  def_prop :Expression.opt, "init"
end

SyntaxFactory.def_syntax :WhileStatement, base: :Statement do
  def_prop :Expression, "test"
  def_prop :Statement, "body"
end

SyntaxFactory.def_syntax :WithStatement, base: :Statement do
  def_prop :Expression, "object"
  def_prop :Statement, "body"
end

SyntaxFactory.def_syntax :YieldExpression, base: :Expression do
  def_prop :Expression.opt, "argument"
  def_prop :Boolean, "delegate"
end

SyntaxFactory.def_syntax :ArrowParameterPlaceHolder, base: :Expression do
  def_prop [:SyntaxNode], "params"
  def_prop :Boolean, "async"
end

SyntaxFactory.def_syntax :JSXClosingElement, base: :SyntaxNode do
  def_prop :SyntaxNode, "name"
end

SyntaxFactory.def_syntax :JSXElement, base: :Expression do
  def_prop :JSXOpeningElement, "openingElement"
  def_prop [:SyntaxNode], "children"
  def_prop :JSXClosingElement.opt, "closingElement"
end

SyntaxFactory.def_syntax :JSXEmptyExpression, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :JSXExpressionContainer, base: :SyntaxNode do
  def_prop :Expression, "expression"
end

SyntaxFactory.def_syntax :JSXIdentifier, base: :SyntaxNode do
  def_prop :String, "name"
end

SyntaxFactory.def_syntax :JSXMemberExpression, base: :SyntaxNode do
  def_prop :SyntaxNode, "object"
  def_prop :JSXIdentifier, "property"
end

SyntaxFactory.def_syntax :JSXAttribute, base: :SyntaxNode do
  def_prop :SyntaxNode, "name"
  def_prop :SyntaxNode.opt, "value"
end

SyntaxFactory.def_syntax :JSXNamespacedName, base: :SyntaxNode do
  def_prop :JSXIdentifier, "namespace_"
  def_prop :JSXIdentifier, "name"
end

SyntaxFactory.def_syntax :JSXOpeningElement, base: :SyntaxNode do
  def_prop :SyntaxNode, "name"
  def_prop :Boolean, "selfClosing"
  def_prop [:SyntaxNode], "attributes"
end

SyntaxFactory.def_syntax :JSXSpreadAttribute, base: :SyntaxNode do
  def_prop :Expression, "argument"
end

SyntaxFactory.def_syntax :JSXText, base: :SyntaxNode do
  def_prop :String, "value"
  def_prop :String, "raw"
end

# TypeScript

SyntaxFactory.def_syntax :TSParameterProperty, base: :SyntaxNode do
  # def_prop :TSAccessibility.opt, "accessibility"
  def_prop :Boolean, "readonly_"
  def_prop :SyntaxNode, "parameter"
end

SyntaxFactory.def_syntax :TSDeclareFunction, base: :Declaration do
  def_prop :Identifier, "id"
  def_prop :Boolean, "decare"
  def_prop :TSTypeAnnotation, "returnType"
end

SyntaxFactory.def_syntax :TSDeclareMethod, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSQualifiedName, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSCallSignatureDeclaration, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSConstructSignatureDeclaration, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSPropertySignature, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSMethodSignature, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSIndexSignature, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSAnyKeyword, base: :TSType do
end

SyntaxFactory.def_syntax :TSBooleanKeyword, base: :TSType do
end

SyntaxFactory.def_syntax :TSBigIntKeyword, base: :TSType do
end

SyntaxFactory.def_syntax :TSNeverKeyword, base: :TSType do
end

SyntaxFactory.def_syntax :TSNullKeyword, base: :TSType do
end

SyntaxFactory.def_syntax :TSNumberKeyword, base: :TSType do
end

SyntaxFactory.def_syntax :TSObjectKeyword, base: :TSType do
end

SyntaxFactory.def_syntax :TSStringKeyword, base: :TSType do
end

SyntaxFactory.def_syntax :TSSymbolKeyword, base: :TSType do
end

SyntaxFactory.def_syntax :TSUndefinedKeyword, base: :TSType do
end

SyntaxFactory.def_syntax :TSUnknownKeyword, base: :TSType do
end

SyntaxFactory.def_syntax :TSVoidKeyword, base: :TSType do
end

SyntaxFactory.def_syntax :TSThisType, base: :TSType do
end

SyntaxFactory.def_syntax :TSFunctionType, base: :TSType do
end

SyntaxFactory.def_syntax :TSConstructorType, base: :TSType do
end

SyntaxFactory.def_syntax :TSTypeReference, base: :TSType do
end

SyntaxFactory.def_syntax :TSTypePredicate, base: :TSType do
end

SyntaxFactory.def_syntax :TSTypeQuery, base: :TSType do
end

SyntaxFactory.def_syntax :TSTypeLiteral, base: :TSType do
end

SyntaxFactory.def_syntax :TSArrayType, base: :TSType do
end

SyntaxFactory.def_syntax :TSTupleType, base: :TSType do
end

SyntaxFactory.def_syntax :TSOptionalType, base: :TSType do
end

SyntaxFactory.def_syntax :TSRestType, base: :TSType do
end

SyntaxFactory.def_syntax :TSUnionType, base: :TSType do
end

SyntaxFactory.def_syntax :TSIntersectionType, base: :TSType do
end

SyntaxFactory.def_syntax :TSConditionalType, base: :TSType do
end

SyntaxFactory.def_syntax :TSInferType, base: :TSType do
end

SyntaxFactory.def_syntax :TSParenthesizedType, base: :TSType do
end

SyntaxFactory.def_syntax :TSTypeOperator, base: :TSType do
end

SyntaxFactory.def_syntax :TSIndexedAccessType, base: :TSType do
end

SyntaxFactory.def_syntax :TSMappedType, base: :TSType do
end

SyntaxFactory.def_syntax :TSLiteralType, base: :TSType do
end

SyntaxFactory.def_syntax :TSExpressionWithTypeArguments, base: :TSType do
end

SyntaxFactory.def_syntax :TSInterfaceDeclaration, base: :Declaration do
end

SyntaxFactory.def_syntax :TSInterfaceBody, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSTypeAliasDeclaration, base: :Declaration do
  def_prop :Identifier, "id"
  def_prop :TSTypeParameterDeclaration.opt, "typeParameters"
  def_prop :TSType, "typeAnnotation"
end

SyntaxFactory.def_syntax :TSAsExpression, base: :Expression do
end

SyntaxFactory.def_syntax :TSTypeAssertion, base: :Expression do
end

SyntaxFactory.def_syntax :TSEnumDeclaration, base: :Declaration do
end

SyntaxFactory.def_syntax :TSEnumMember, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSModuleDeclaration, base: :Declaration do
end

SyntaxFactory.def_syntax :TSModuleBlock, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSImportType, base: :TSType do
end

SyntaxFactory.def_syntax :TSImportEqualsDeclaration, base: :Declaration do
end

SyntaxFactory.def_syntax :TSExternalModuleReference, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSNonNullExpression, base: :Expression do
end

SyntaxFactory.def_syntax :TSExportAssignment, base: :Statement do
end

SyntaxFactory.def_syntax :TSNamespaceExportDeclaration, base: :Statement do
end

SyntaxFactory.def_syntax :TSTypeAnnotation, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSTypeParameterInstantiation, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSTypeParameterDeclaration, base: :SyntaxNode do
end

SyntaxFactory.def_syntax :TSTypeParameter, base: :SyntaxNode do
end
