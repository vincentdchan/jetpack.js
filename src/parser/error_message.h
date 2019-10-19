//
// Created by Duzhong Chen on 2019/9/20.
//
#pragma once

namespace ParseMessages {
    
    static const char* BadImportCallArity = "Unexpected token";
    static const char* BadGetterArity = "Getter must not have any formal parameters";
    static const char* BadSetterArity = "Setter must have exactly one formal parameter";
    static const char* BadSetterRestParameter = "Setter function argument must not be a rest parameter";
    static const char* ConstructorIsAsync = "Class constructor may not be an async method";
    static const char* ConstructorSpecialMethod = "Class constructor may not be an accessor";
    static const char* DeclarationMissingInitializer = "Missing initializer in %0 declaration";
    static const char* DefaultRestParameter = "Unexpected token =";
    static const char* DefaultRestProperty = "Unexpected token =";
    static const char* DuplicateBinding = "Duplicate binding %0";
    static const char* DuplicateConstructor = "A class may only have one constructor";
    static const char* DuplicateProtoProperty = "Duplicate __proto__ fields are not allowed in object literals";
    static const char* ForInOfLoopInitializer = "%0 loop variable declaration may not have an initializer";
    static const char* GeneratorInLegacyContext = "Generator declarations are not allowed in legacy contexts";
    static const char* IllegalBreak = "Illegal break statement";
    static const char* IllegalContinue = "Illegal continue statement";
    static const char* IllegalExportDeclaration = "Unexpected token";
    static const char* IllegalImportDeclaration = "Unexpected token";
    static const char* IllegalLanguageModeDirective = "Illegal \"use strict\" directive in function with non-simple parameter list";
    static const char* IllegalReturn = "Illegal return statement";
    static const char* InvalidEscapedReservedWord = "Keyword must not contain escaped characters";
    static const char* InvalidHexEscapeSequence = "Invalid hexadecimal escape sequence";
    static const char* InvalidLHSInAssignment = "Invalid left-hand side in assignment";
    static const char* InvalidLHSInForIn = "Invalid left-hand side in for-in";
    static const char* InvalidLHSInForLoop = "Invalid left-hand side in for-loop";
    static const char* InvalidModuleSpecifier = "Unexpected token";
    static const char* InvalidRegExp = "Invalid regular expression";
    static const char* LetInLexicalBinding = "let is disallowed as a lexically bound name";
    static const char* MissingFromClause = "Unexpected token";
    static const char* MultipleDefaultsInSwitch = "More than one default clause in switch statement";
    static const char* NewlineAfterThrow = "Illegal newline after throw";
    static const char* NoAsAfterImportNamespace = "Unexpected token";
    static const char* NoCatchOrFinally = "Missing catch or finally after try";
    static const char* ParameterAfterRestParameter = "Rest parameter must be last formal parameter";
    static const char* PropertyAfterRestProperty = "Unexpected token";
    static const char* Redeclaration = "%0 \"%1\" has already been declared";
    static const char* StaticPrototype = "Classes may not have static property named prototype";
    static const char* StrictCatchVariable = "Catch variable may not be eval or arguments in strict mode";
    static const char* StrictDelete = "Delete of an unqualified identifier in strict mode.";
    static const char* StrictFunction = "In strict mode code, functions can only be declared at top level or inside a block";
    static const char* StrictFunctionName = "Function name may not be eval or arguments in strict mode";
    static const char* StrictLHSAssignment = "Assignment to eval or arguments is not allowed in strict mode";
    static const char* StrictLHSPostfix = "Postfix increment/decrement may not have eval or arguments operand in strict mode";
    static const char* StrictLHSPrefix = "Prefix increment/decrement may not have eval or arguments operand in strict mode";
    static const char* StrictModeWith = "Strict mode code may not include a with statement";
    static const char* StrictOctalLiteral = "Octal literals are not allowed in strict mode.";
    static const char* StrictParamDupe = "Strict mode function may not have duplicate parameter names";
    static const char* StrictParamName = "Parameter name eval or arguments is not allowed in strict mode";
    static const char* StrictReservedWord = "Use of future reserved word in strict mode";
    static const char* StrictVarName = "Variable name may not be eval or arguments in strict mode";
    static const char* TemplateOctalLiteral = "Octal literals are not allowed in template strings.";
    static const char* UnexpectedEOS = "Unexpected end of input";
    static const char* UnexpectedIdentifier = "Unexpected identifier";
    static const char* UnexpectedNumber = "Unexpected number";
    static const char* UnexpectedReserved = "Unexpected reserved word";
    static const char* UnexpectedString = "Unexpected string";
    static const char* UnexpectedTemplate = "Unexpected quasi %0";
    static const char* UnexpectedToken = "Unexpected token {}";
    static const char* UnexpectedTokenIllegal = "Unexpected token ILLEGAL";
    static const char* UnknownLabel = "Undefined label \"%0\"";
    static const char* UnterminatedRegExp = "Invalid regular expression: missing /";
    
}

