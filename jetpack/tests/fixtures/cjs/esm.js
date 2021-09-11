const { someMethod } = require('./cjs');

export function printHelloWorld() {
    console.log('hello world', someMethod());
}
