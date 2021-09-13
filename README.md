# Jetpack.js

[![Build Status](https://travis-ci.com/vincentdchan/jetpack.js.svg?branch=master)](https://travis-ci.com/vincentdchan/jetpack.js)
[![npm version](https://img.shields.io/npm/v/jetpp.svg)](https://www.npmjs.com/package/jetpp)

| [中文版](./README_CN.md) | [WASM Online Demo](https://diverse.space/jetpack-wasm-demo/)

`jetpack.js` is an extremely fast js bundler and minifier written in C++.

- [Features](#Features)
  - [Parser](#parser)
  - [Bundler](#bundler)
- [Installation](#Installation)
- [Usage](#usage)
- [WebAssembly User](#webAssembly-user)
- [Platform](#platform)

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
- Constant folding.
- Minify the code.
- Sourcemap generation

# Installation & Usage

## Binary Distribution

Download the binary from the [release](https://github.com/vincentdchan/jetpack.js/releases) page.

Usage:

```shell
jetpack-cli --help
```

## Node.js CLI

```
npm install -g jetpp
```


Use command line to bundle a js module.
```shell script
jetpp main.js --out bundle.js
```

Help command:

```shell script
$ jetpp --help

Jetpack command line
Usage:
  Jetpack [OPTION...] positional parameters

      --tolerant            tolerant parsing error
      --jsx                 support jsx syntax
      --library             bundle as library, do not bundle node_modules
      --help                produce help message
      --analyze-module arg  analyze a module and print result
      --no-trace            do not trace ref file when analyze module
      --minify              minify the code
      --out arg             output filename of bundle
      --sourcemap           generate sourcemaps
```

## Node.js Program

```javascript

const jetpp = require('jetpp');

console.log(jetpp.minify('let hello = "world";'));

```

# WebAssembly User

WASM gives you the power of running Jetpack.js in the browser environment.

## Install the WASM version

```
yarn add jetpp-wasm
```

## Include Jetpack.js in your project

```javascript

import loadJetpack from 'jetpp-wasm';

async function main(code) {
    const jetpack = await loadJetpack();
    console.log(jetpack.minify(code));  // minify
    console.log(jetpack.parse(code));  // parse
}

```

# Platform

`jetpack.js` supports all popular system including:

- macOS x64/arm64
- Windows 64bit
- Linux 64bit
- WebAssembly

# Build Dependencies

- jemalloc 5.2.1
- cxxopts
- fmt
- nlohmann_json
- robin-hood-hashing 3.11.1
- xxHash
- boost(Header-only) 1.76
