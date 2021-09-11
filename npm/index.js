#!/usr/bin/env node

let bin;
if (process.env['JETPACK_LOCAL']) {
  bin = require('./build/Release/jetpp.node');
} else {
  bin = require('./jetpp.node');
}

module.exports = bin;
