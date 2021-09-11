const fs = require('fs')
const path = require('path');

const fileList = [
  'jetpack-wasm.js',
  'jetpack-wasm.wasm',
];

const buildDir = '../cmake-build-release-wasm/jetpack-wasm';

function copyReadme() {
  const name = 'README.md';
  const filePath = '../README.md';

  try {
    fs.unlinkSync(name);
  } catch (e) {}
  fs.copyFileSync(filePath, name);
}

function main() {
  fileList.forEach(name => {
    const filePath = path.join(buildDir, name);
    try {
      fs.unlinkSync(name);
    } catch (e) {}
    fs.copyFileSync(filePath, name);
  });

  copyReadme();
}

main();
