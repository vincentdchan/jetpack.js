# Jetpack.js

[![Build Status](https://travis-ci.com/vincentdchan/jetpack.js.svg?branch=master)](https://travis-ci.com/vincentdchan/jetpack.js)
[![npm version](https://img.shields.io/npm/v/jetpp.svg)](https://www.npmjs.com/package/jetpp)

[English Version](./README.md)

`jetpack.js` 是一个超级快的 ECMAScript 打包和压缩工具，用 C++ 写成。

- [特性](#特性)
- [安装](#Installation)
- [使用](#usage)
- [WebAssembly 用户](#WebAssembly-用户)
- [平台](#platform)

# 特性

- 可以独立运行，可以作为 Node.js 模块
- 使用 C++ 实现，拥有极佳性能
- 完整支持 ECMAScript 2017([ECMA-262 8th Edition](http://www.ecma-international.org/publications/standards/Ecma-262.htm))
- 支持 [JSX](https://facebook.github.io/jsx/)， 一种 [React](https://facebook.github.io/react/) 的语法
- 支持语法节点信息
- 友好的错误提示
- 打包成一个文件
- Scope hoisting
- 常量折叠
- 压缩代码
- Sourcemap 生成

# 安装

```
npm install -g jetpp
```

Or

```
yarn global add jetpp
```

# 使用

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

# WebAssembly 用户

WASM 让你可以在浏览器环境里面运行 Jetpack.js.

## 安装 WASM 版 Jetpack

```
yarn add jetpp-wasm
```

## 引入 Jetpack.js 到你的项目里面

```javascript

import loadJetpack from 'jetpp-wasm';

async function main(code) {
    const jetpack = await loadJetpack();
    return jetpack.minify(code);
}

```

# 平台

`jetpack.js` 支持常见的平台:

- macOS
- Windows 64bit
- Linux 64bit
- WebAssembly

# 构建依赖
- cxxopts
- fmt
- nlohmann_json
- robin-hood-hashing 3.11.1
- boost(头文件) 1.76
