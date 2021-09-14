const jetpp = require('./index');

test('minify', () => {
	expect(jetpp.minify('let a = 333')).toBe('let q=333;');
});
