# Jetpack++

`jetpack.js` is an extremely fast js bundler and minifier.

`jetpack.js` a well designed tool. It's modulize into parser and bundler.
The parser can be used as a library independently.

- [Why](#why)
- [Features](#Features)
  - [Parser](#parser)
  - [Bundler](#bundler)
- [Installation](#Installation)
- [Usage](#usage)
- [Use the parser as a library](#use-the-parser-as-a-library)
  - [Example](#example)
- [Performance](#performance)
- [Architecture](#architecture)
- [Compatibility](#compatibility)
- [Platform](#platform)

# Why

Several months ago, I wrote a js parser in C++. Many parsers embedded in other js engine have a lot of dependencies on their runtime. It's very hard to separate them. So I decided to write my own parser. After I finished, I found my parser is almost 10x faster than the common parsers running on Node.js (without heating). But I didn't move on after that. This year(2020), Evan Wallace's project [esbuild](https://github.com/evanw/esbuild/) inspires me. So I decided to write a bundler in C++. I want to know if Golang's performance could beat C++.

# Features

## Parser

- Can be used standalone
- Implemented in C++ with excellent performance
- Full support for ECMAScript 2017([ECMA-262 8th Edition](http://www.ecma-international.org/publications/standards/Ecma-262.htm))
- JSON output of sensible [syntax tree](https://github.com/estree/estree/blob/master/es5.md) format as standardized by [ESTree project](https://github.com/estree/estree)
- Experimental support for [JSX](https://facebook.github.io/jsx/), a syntax extension for [React](https://facebook.github.io/react/)
- Syntax node location (index-based and line-column)
- Friendly error message

## Bundler

- Module resolution.
- Bundle a ES project into a single file.
- Scope hoisting.
- Contant folding.
- Minify the code.

# Installation

```
npm install -g jetpackpp
```

Or

```
yarn global add jetpackpp
```

# Usage

Use command line to bundle a js module.
```shell script
jetpack main.js --out bundle.js
```

Help command:

```shell script
$ jetpackpp --help

$ Jetpack++ command line
Usage:
  Jetpack++ [OPTION...] positional parameters

      --tolerant            tolerant parsing error
      --jsx                 support jsx syntax
      --help                produce help message
      --analyze-module arg  analyze a module and print result
      --no-trace            do not trace ref file when analyze module
      --minify              minify the code
      --out arg             output filename of bundle
```

# Use the parser as a standalone library

jetpack.js is built with CMake, so it can be
easily integrated to your project.

```cmake
add_subdirectory(js-parser)
target_include_directories(${PROJECT_NAME} ./js-parser/src)
target_link_libraries(${PROJECT_NAME} PUBLIC js-parser)
```

# Performance

I do the same benchmark provided by [esbuild](https://github.com/evanw/esbuild).

> My main benchmark approximates a large codebase by duplicating the three.js library 10 times and building a single bundle from scratch, without any caches. For this benchmark, esbuild is 10-100x faster than the other JavaScript bundlers I tested (Webpack, Rollup, Parcel, and FuseBox). The benchmark can be run with make bench-three.

![](./images/chart.svg)

The tests were done on a 6-core 2018 MacBook Pro with 16GB of RAM
(similar to esbuild).

# Architecture

![](./images/Rocket-Bundle-Arch.png)

The code are well commented, please read the code.

# Platform

`jetpack.js` supports all popular system including:

- macOS
- Windows 64bit
- Linux 64bit
