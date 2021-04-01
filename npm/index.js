#!/usr/bin/env node

const bin = require('./build/Release/jetpp.node');

const retValue = bin.handleCli(process.argv.slice(1));

process.exit(retValue);
