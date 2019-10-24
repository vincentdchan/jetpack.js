# Zhong's ECMAScript Parser

ZEP(Zhong's ES Parser) is a ECMAScript parser
implemented in C++ aimed at excellent performance and
scalability.

# Features

- Implemented in C++ with excellent performance
- Full support for ECMAScript 2017([ECMA-262 8th Edition](http://www.ecma-international.org/publications/standards/Ecma-262.htm))
- JSON output of sensible [syntax tree](https://github.com/estree/estree/blob/master/es5.md) format as standardized by [ESTree project](https://github.com/estree/estree)
- Experimental support for [JSX](https://facebook.github.io/jsx/), a syntax extension for [React](https://facebook.github.io/react/)
- Syntax node location (index-based and line-column)

# WIP Features

- [ ] TypeScript Support.
- [ ] Refactor AST with C++ template, better to do type-checking and static anslysis.

# Usage

ZEP contains a cli tool and a C++ library.

With cli tool you can parse ES file in
command line.

With C++ library you can use ZEP to parse
ES source code in your project.

## Command line

Parse a ES file as a script:

```shell script
zep --entry=./test.js
```

Parse a ES file as a module:

```shell script
zep --entry=./test.js --es-module
```

Parse a jsx file as a module:

```shell script
zep --entry=./test.js --es-module --jsx
```

Dump AST as a file:

```shell script
zep --entry=./test.js >> ast.json
```

Help:
```shell script
zep --help
```

## Use ZEP as a library

Please install jemalloc and boost on your mac:

```shell script
brew install jemalloc boost
```

ZEP is built with CMake, so it can be
easily integrated to your project.

```cmake
add_subdirectory(ZEP)
target_include_directories(${PROJECT_NAME} ./ZEP/src)
target_link_libraries(${PROJECT_NAME} zep)
```

### Example

```cpp
ParserCommon::Config config = ParserCommon::Config::Default();
Parser parser(src, config);

auto script = parser.ParseScript();
// or
auto module = parser.ParseModule();

```

# Performance

ZEP(release version)'s parsing speed would be nearly 1x faster than
other ES parsers implemented in ES(Running on Node.js).

With the power of jemalloc,
ZEP's performance is equal to other ES parsers implemented in Rust.

# Compatibility

The json output of ZEP would as same as esprima. So I think maybe ZEP can be
a faster alternative to some ES parsers.

And the WASM version is in the roadmap. The web version of ZEP would be released ASAP.

# Platform

ZEP can only run on macOS currently.
The Linux version would be released
as soon as possible. Windows version
is in the roadmap, which is considered
low priority.
