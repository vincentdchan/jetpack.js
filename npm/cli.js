#!/usr/bin/env node

const bin = require('./index');

const retValue = bin.handleCli(process.argv.slice(1));

process.exit(retValue);
