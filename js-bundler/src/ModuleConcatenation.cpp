//
// Created by Duzhong Chen on 2020/3/24.
//

#include "ModuleConcatenation.h"

/**
 * Reference: https://github.com/evanw/esbuild/blob/master/src/esbuild/bundler/bundler.go
 */
static const char* BOOTSTRAP_CODE =
    "((modules, entryPoint) => {\n"
    "\t\t\tlet global = function() { return this }()\n"
    "\t\t\tlet cache = {}\n"
    "\t\t\tlet require = (target, arg) => {\n"
    "\t\t\t\t// If the first argument is a number, this is an import\n"
    "\t\t\t\tif (typeof target === 'number') {\n"
    "\t\t\t\t\tlet module = cache[target], exports\n"
    "\t\t\t\t\t// Evaluate the module if needed\n"
    "\t\t\t\t\tif (!module) {\n"
    "\t\t\t\t\t\tmodule = cache[target] = {exports: {}}\n"
    "\t\t\t\t\t\tmodules[target].call(global, require, module.exports, module)\n"
    "\t\t\t\t\t}\n"
    "\t\t\t\t\t// Return the exports object off the module in case it was overwritten\n"
    "\t\t\t\t\texports = module.exports\n"
    "\t\t\t\t\t// Convert CommonJS exports to ES6 exports\n"
    "\t\t\t\t\tif (arg && (!exports || !exports.__esModule)) {\n"
    "\t\t\t\t\t\tif (!exports || typeof exports !== 'object') {\n"
    "\t\t\t\t\t\t\texports = {}\n"
    "\t\t\t\t\t\t}\n"
    "\t\t\t\t\t\tif (!('default' in exports)) {\n"
    "\t\t\t\t\t\t\tObject.defineProperty(exports, 'default', {\n"
    "\t\t\t\t\t\t\t\tget: () => module.exports,\n"
    "\t\t\t\t\t\t\t\tenumerable: true,\n"
    "\t\t\t\t\t\t\t})\n"
    "\t\t\t\t\t\t}\n"
    "\t\t\t\t\t}\n"
    "\t\t\t\t\treturn exports\n"
    "\t\t\t\t}\n"
    "\t\t\t\t// Mark this module as an ES6 module using a non-enumerable property\n"
    "\t\t\t\tObject.defineProperty(target, '__esModule', {\n"
    "\t\t\t\t\tvalue: true,\n"
    "\t\t\t\t})\n"
    "\t\t\t\tfor (let name in arg) {\n"
    "\t\t\t\t\tObject.defineProperty(target, name, {\n"
    "\t\t\t\t\t\tget: arg[name],\n"
    "\t\t\t\t\t\tenumerable: true,\n"
    "\t\t\t\t\t})\n"
    "\t\t\t\t}\n"
    "\t\t\t}\n"
    "\t\t\treturn require(entryPoint)\n"
    "\t\t})";

namespace rocket_bundle {

    const char* ModuleConcatenation::GetBootstrapCode() {
        return BOOTSTRAP_CODE;
    }

}
