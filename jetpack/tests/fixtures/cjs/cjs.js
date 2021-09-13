const cjs2 = require('./cjs2');
const fs = require('fs');

exports.someMethod = function () {
    const content = fs.readFileSync('./test');
    console.log('hello ', cjs2.method2(), content);
}
