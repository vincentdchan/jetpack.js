#!/usr/bin/env node

const cp = require('child_process');
const os = require('os');
const path = require('path');
const fs = require('fs');

function spawnExe(path) {
  try {
    fs.accessSync(path, fs.constants.X_OK);
  } catch (err) {
    const stat = fs.statSync(path);
    fs.chmodSync(path, stat.mode | 0700);
  }
  const child = cp.spawn(path, process.argv, {
    stdio: ['inherit', 'inherit', 'inherit']
  });
  child.on('exit', (code) => {
    process.exit(code);
  });
}

switch (os.platform()) {
  case 'darwin':
  case 'linux': {
    const execPath = path.join(__dirname, `./bin/${os.platform()}/jetpack`);
    spawnExe(execPath);
    break;
  }

  case 'win32': {
    const execPath = path.join(__dirname, `./bin/${os.platform()}/jetpack.exe`);
    spawnExe(execPath);
    break;
  }

  default:
    console.error('Unknown platform: ', os.platform);
    process.exit(255);

}
