#lang racket

(provide 
  syntax-list
  (struct-out syntax-node)
)

(struct syntax-node (
  id
  base
  props
))

(define (Vec id)
  (cons 'Vec id)
)

(define (Option id)
  (cons 'Option id)
)

(define (Variant args)
  (cons 'Variant args)
)

(define (virtual name)
  (cons 'Virtual name)
)

;;; (define a (syntax-node 'haha (list) (list)))
(define syntax-list (list

  (syntax-node (virtual 'SyntaxNode) null
    (list)
  )

  (syntax-node (virtual 'Expression) 'SyntaxNode
    (list)
  )

  (syntax-node (virtual 'Statement) 'SyntaxNode
    (list)
  )

  (syntax-node (virtual 'Pattern) 'SyntaxNode
    (list)
  )

  (syntax-node (virtual 'Declaration) 'SyntaxNode
    (list)
  )

  (syntax-node 'ArrayExpression 'Expression
    (list
      (cons (Vec 'SyntaxNode) "elements")
    )
  )

  (syntax-node 'ArrayPattern 'SyntaxNode
    (list
      (cons (Vec 'SyntaxNode) "elements")
    )
  )

  (syntax-node 'ArrowFunctionExpression 'Expression
    (list
      (cons (Option 'Identifier) "id")
      (cons (Vec 'SyntaxNode) "params")
      (cons 'Expression "body")
      (cons 'Boolean "generator")
      (cons 'Boolean "expression")
      (cons 'Boolean "async")
    )
  )

  (syntax-node 'AssignmentExpression 'Expression
    (list
      (cons 'String "operator")
      (cons 'Expression "left")
      (cons 'Expression "right")
    )
  )

  (syntax-node 'AssignmentPattern 'SyntaxNode
    (list
      (cons 'SyntaxNode "left")
      (cons 'Expression "right")
    )
  )

  (syntax-node 'AsyncArrowFunctionExpression 'Expression
    (list
      (cons (Option 'Identifier) "id")
      (cons (Vec 'SyntaxNode) "params")
      (cons 'Expression "body")
      (cons 'Boolean "generator")
      (cons 'Boolean "expression")
      (cons 'Boolean "async")
    )
  )

  (syntax-node 'AsyncFunctionDeclaration 'Declaration
    (list
      (cons (Option 'Identifier) "id")
      (cons (Vec 'SyntaxNode) "params")
      (cons 'BlockStatement "body")
      (cons 'Boolean "generator")
      (cons 'Boolean "expression")
      (cons 'Boolean "async")
    )
  )

  (syntax-node 'AsyncFunctionExpression 'Expression
    (list
      (cons (Option 'Identifier) "id")
      (cons (Vec 'SyntaxNode) "params")
      (cons 'Expression "body")
      (cons 'Boolean "generator")
      (cons 'Boolean "expression")
      (cons 'Boolean "async")
    )
  )

  (syntax-node 'AwaitExpression 'Expression
    (list
      (cons 'Expression "argument")
    )
  )

  (syntax-node 'BinaryExpression 'Expression
    (list
      (cons 'String "operator")
      (cons 'Expression "left")
      (cons 'Expression "right")
    )
  )

  (syntax-node 'BlockStatement 'Statement
    (list
      (cons (Vec 'Statement) "body")
    )
  )

  (syntax-node 'BreakStatement 'Statement
    (list
      (cons (Option 'Identifier) "label")
    )
  )

  (syntax-node 'CallExpression 'Expression
    (list
      (cons 'Expression "callee")
      (cons (Vec 'SyntaxNode) "arguments")
    )
  )

  (syntax-node 'CatchClause 'SyntaxNode
    (list
      (cons 'SyntaxNode "param")
      (cons 'BlockStatement "body")
    )
  )

  (syntax-node 'ClassBody 'SyntaxNode
    (list
      (cons (Vec 'Property) "body")
    )
  )

  (syntax-node 'ClassDeclaration 'Declaration
    (list
      (cons (Option 'Identifier) "id")
      (cons (Option 'Identifier) "superClass")
      (cons 'ClassBody "body")
    )
  )

  (syntax-node 'ClassExpression 'Expression
    (list
      (cons (Option 'Identifier) "id")
      (cons (Option 'Identifier) "superClass")
      (cons (Option 'ClassBody) "body")
    )
  )

  (syntax-node 'ComputedMemberExpression 'Expression
    (list
      (cons 'Boolean "computed")
      (cons 'Expression "object")
      (cons 'Expression "property")
    )
  )

  (syntax-node 'ConditionalExpression 'Expression
    (list
      (cons 'Expression "test")
      (cons 'Expression "consequent")
      (cons 'Expression "alternate")
    )
  )

  (syntax-node 'ContinueStatement 'Statement
    (list
      (cons (Option 'Identifier) "label")
    )
  )

  (syntax-node 'DebuggerStatement 'SyntaxNode
    (list)
  )

  (syntax-node 'Directive 'SyntaxNode
    (list
      (cons 'Expression "expression")
      (cons 'String "directive")
    )
  )

  (syntax-node 'DoWhileStatement 'Statement
    (list
      (cons 'Statement "body")
      (cons 'Expression "test")
    )
  )

  (syntax-node 'EmptyStatement 'SyntaxNode
    (list)
  )

  (syntax-node 'ExportAllDeclaration 'Declaration
    (list
      (cons 'Literal "source")
    )
  )

  (syntax-node 'ExportDefaultDeclaration 'Declaration
    (list
      (cons 'SyntaxNode "declaration")
    )
  )

  (syntax-node 'ExportNamedDeclaration 'Declaration
    (list
      (cons (Option 'SyntaxNode) "declaration")
      (cons (Vec 'ExportSpecifier) "specifiers")
      (cons (Option 'Literal) "source")
    )
  )

  (syntax-node 'ExportSpecifier 'SyntaxNode
    (list
      (cons 'Identifier "exported")
      (cons 'Identifier "local")
    )
  )

  (syntax-node 'ExpressionStatement 'Statement
    (list
      (cons 'Expression "expression")
    )
  )

  (syntax-node 'ForInStatement 'Statement
    (list
      (cons 'Expression "left")
      (cons 'Expression "right")
      (cons 'Statement "body")
      (cons 'Boolean "each")
    )
  )

  (syntax-node 'ForOfStatement 'Statement
    (list
      (cons 'Expression "left")
      (cons 'Expression "right")
      (cons 'Statement "body")
    )
  )

  (syntax-node 'ForStatement 'Statement
    (list
      (cons (Option 'Expression) "init")
      (cons (Option 'Expression) "test")
      (cons (Option 'Expression) "update")
      (cons 'Statement "body")
    )
  )

  (syntax-node 'FunctionDeclaration 'Declaration
    (list
      (cons (Option 'Identifier) "id")
      (cons (Vec 'SyntaxNode) "params")
      (cons 'BlockStatement "body")
      (cons 'Boolean "generator")
      (cons 'Boolean "expression")
      (cons 'Boolean "async")
    )
  )

  (syntax-node 'FunctionExpression 'Expression
    (list
      (cons (Option 'Identifier) "id")
      (cons (Vec 'SyntaxNode) "params")
      (cons 'BlockStatement "body")
      (cons 'Boolean "generator")
      (cons 'Boolean "expression")
      (cons 'Boolean "async")
    )
  )

  (syntax-node 'Identifier 'Expression
    (list
      (cons 'String "name")
    )
  )

  (syntax-node 'IfStatement 'Statement
    (list
      (cons 'Expression "test")
      (cons 'Statement "consequent")
      (cons (Option 'Statement) "alternate")
    )
  )

  (syntax-node 'Import 'SyntaxNode
    (list)
  )

  (syntax-node 'ImportDeclaration 'Declaration
    (list
      (cons (Vec 'SyntaxNode) "specifiers")
      (cons 'Literal "source")
    )
  )

  (syntax-node 'ImportDefaultSpecifier 'SyntaxNode
    (list
      (cons 'Identifier "local")
    )
  )

  (syntax-node 'ImportNamespaceSpecifier 'SyntaxNode
    (list
      (cons 'Identifier "local")
    )
  )

  (syntax-node 'ImportSpecifier 'SyntaxNode
    (list
      (cons 'Identifier "local")
      (cons 'Identifier "imported")
    )
  )

  (syntax-node 'LabeledStatement 'Statement
    (list
      (cons 'Identifier "label")
      (cons 'Statement "body")
    )
  )

  (syntax-node 'Literal 'SyntaxNode
    (list
      (cons (Variant (list 'Boolean 'Number 'String)) "value")
      (cons 'String "raw")
    )
  )

  (syntax-node 'MetaProperty 'SyntaxNode
    (list
      (cons 'Identifier "meta")
      (cons 'Identifier "property")
    )
  )

  (syntax-node 'MethodDefinition 'SyntaxNode
    (list
      (cons (Option 'Expression) "key")
      (cons 'Boolean "computed")
      (cons (Option 'Expression) "value")
      (cons 'String "kind")
      (cons 'Boolean "static")
    )
  )

  (syntax-node 'Module 'SyntaxNode
    (list
      (cons (Vec 'SyntaxNode) "body")
      (cons 'String "sourceType")
    )
  )

  (syntax-node 'NewExpression 'Expression
    (list
      (cons 'Expression "callee")
      (cons (Vec 'SyntaxNode) "arguments")
    )
  )

  (syntax-node 'ObjectExpression 'Expression
    (list
      (cons (Vec 'SyntaxNode) "properties")
    )
  )

  (syntax-node 'ObjectPattern 'Pattern
    (list
      (cons (Vec 'SyntaxNode) "properties")
    )
  )

  (syntax-node 'Property 'SyntaxNode
    (list
      (cons 'SyntaxNode "key")
      (cons 'Boolean "computed")
      (cons (Option 'SyntaxNode) "value")
      (cons 'String "kind")
      (cons 'Boolean "method")
      (cons 'Boolean "shorthand")
    )
  )

  (syntax-node 'RegexLiteral 'SyntaxNode
    (list
      (cons 'String "value")
      (cons 'String "raw")
    )
  )

  (syntax-node 'RestElement 'SyntaxNode
    (list
      (cons 'SyntaxNode "argument")
    )
  )

  (syntax-node 'ReturnStatement 'Statement
    (list
      (cons (Option 'Expression) "argument")
    )
  )

  (syntax-node 'Script 'SyntaxNode
    (list
      (cons (Vec 'SyntaxNode) "body")
      (cons 'String "sourceType")
    )
  )

  (syntax-node 'SequenceExpression 'Expression
    (list
      (cons (Vec 'Expression) "expressions")
    )
  )

  (syntax-node 'SpreadElement 'SyntaxNode
    (list
      (cons 'Expression "argument")
    )
  )

  (syntax-node 'StaticMemberExpression 'Expression
    (list
      (cons 'Boolean "computed")
      (cons 'Expression "object")
      (cons 'Expression "property")
    )
  )

  (syntax-node 'Super 'SyntaxNode
    (list)
  )

  (syntax-node 'SwitchCase 'SyntaxNode
    (list
      (cons (Option 'Expression) "test")
      (cons (Vec 'Statement) "consequent")
    )
  )

  (syntax-node 'SwitchStatement 'Statement
    (list
      (cons 'Expression "discrimiant")
      (cons (Vec 'SwitchCase) "cases")
    )
  )

  (syntax-node 'TaggedTemplateExpression 'Expression
    (list
      (cons 'Expression "tag")
      (cons 'TemplateElement "quasi")
    )
  )

  (syntax-node 'TemplateElement 'SyntaxNode
    (list
      (cons (Vec 'TemplateElement) "quasis")
      (cons (Vec 'Expression) "expressions")
    )
  )

  (syntax-node 'ThisExpression 'Expression
    (list)
  )

  (syntax-node 'ThrowStatement 'Statement
    (list
      (cons 'Expression "argument")
    )
  )

  (syntax-node 'TryStatement 'Statement
    (list
      (cons 'BlockStatement "block")
      (cons (Option 'CatchClause) "handler")
      (cons (Option 'BlockStatement) "finalizer")
    )
  )

  (syntax-node 'UnaryExpression 'Expression
    (list
      (cons 'String "operator")
      (cons 'Expression "argument")
      (cons 'Boolean "prefix")
    )
  )

  (syntax-node 'UpdateExpression 'Expression
    (list
      (cons 'String "operator")
      (cons 'Expression "argument")
      (cons 'Boolean "prefix")
    )
  )

  (syntax-node 'VariableDeclaration 'Declaration
    (list
      (cons (Vec 'VariableDeclarator) "declarations")
      (cons 'String "kind")
    )
  )

  (syntax-node 'VariableDeclarator 'SyntaxNode
    (list
      (cons 'SyntaxNode "id")
      (cons (Option 'Expression) "init")
    )
  )

  (syntax-node 'WhileStatement 'Statement
    (list
      (cons 'Expression "test")
      (cons 'Statement "body")
    )
  )

  (syntax-node 'WithStatement 'Statement
    (list
      (cons 'Expression "object")
      (cons 'Statement "body")
    )
  )

  (syntax-node 'YieldExpression 'Expression
    (list
      (cons (Option 'Expression) "argument")
      (cons 'Boolean "delegate")
    )
  )

))
