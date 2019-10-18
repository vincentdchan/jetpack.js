# Zhong's ECMAScript Parser

ZEP(Zhong's ES Parser) is a ECMAScript parser
implemented in C++ aimed for excellent performance and
scalability.

# Usage

ZEP contains a cli tool and a C++ library.

With cli tool you can parse ES file in
command line.

With C++ library you can use ZEP to parse
ES source code in your project.

## Command line

Parse a ES file:

```shell script
zep --entry=./test.js
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

# Platform

ZEP can only run on macOS currently.
The Linux version would be released
as soon as possible. Windows version
is in the roadmap, which is considered
low priority.
