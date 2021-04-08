
const https = require('https');
const fs = require('fs');
const path = require('path');
const os = require('os');
const crypto = require('crypto');

const version = '0.1.0';

const platform = os.platform();
const arch = os.arch();
const downloadUrl = name => `https://www.polodb.org/jetpack/${version}/node/${platform}/${arch}/${name}`;
const downloadChecksumUrl = name => `${downloadUrl(name)}.SHA256`;

const libsName = [
  'jetpp.node',
];

if (platform === 'darwin') {
  libsName.push('libjetpackd.dylib');
  libsName.push('libjemalloc.2.dylib');
}

function getDownloadPath(name) {
  const tmpDir = os.tmpdir();
  const projectDir = path.join(tmpDir, version, 'jetpack-node');
  if (!fs.existsSync(projectDir)) {
    fs.mkdirSync(projectDir, {
      recursive: true,
    });
  }
  const nodeFilePath = path.join(projectDir, name);
  return nodeFilePath;
}

function downloadChecksumFile(name) {
  return new Promise((resolve, reject) => {
    const checksumPath = getDownloadPath(name) + '.SHA256';
    const file = fs.createWriteStream(checksumPath);
    https.get(downloadChecksumUrl(name), function(resp) {
      resp.pipe(file);
      resp.on('error', err => {
        reject(err);
      });
      resp.on('end', () => {
        if (resp.complete) {
          resolve(checksumPath);
        }
      });
    });

  });
}

function downloadLib(url, path) {
  return new Promise((resolve, reject) => {
    const file = fs.createWriteStream(path);
    https.get(url, function (resp) {
      resp.pipe(file);
      resp.on('error', err => {
        reject(err);
      });
      resp.on('end', () => {
        if (resp.complete) {
          resolve();
        }
      })
    });
  });
}

function calsha256(filename) {
  return new Promise((resolve, reject) => {
    const sum = crypto.createHash('sha256');
    const fileStream = fs.createReadStream(filename);
    fileStream.on('error', function (err) {
      return reject(err)
    });
    fileStream.on('data', function (chunk) {
      try {
        sum.update(chunk);
      } catch (ex) {
        return reject(ex);
      }
    });
    fileStream.on('end', function () {
      return resolve(sum.digest('hex'))
    });
  });
};

async function checksum(checksumFilePath, libPath) {
  let checksumContent = fs.readFileSync(checksumFilePath, 'utf-8');
  let actualChecksum = await calsha256(libPath);
  return checksumContent === actualChecksum;
}

async function main() {
  try {
    const promises = libsName.map(async (name) => {
      const checksumPath = await downloadChecksumFile(name);
      const nodeFilePath = getDownloadPath(name);
      console.log('Jetpack lib path: ', nodeFilePath);
      if (!fs.existsSync(nodeFilePath)) {
        console.log('lib not found, begin to download from: ', downloadUrl(name));
        await downloadLib(downloadUrl(name), nodeFilePath);
      }

      if (!await checksum(checksumPath, nodeFilePath)) {
        console.log('checksum mismatch');
        process.exit(-1);
      }

      copyNodeToDest(nodeFilePath, name);
    });

    await Promise.all(promises);
  } catch (err) {
    console.error(err);
    process.exit(-1);
  }
}

function copyNodeToDest(nodeFilePath, name) {
  fs.copyFileSync(nodeFilePath, path.join(__dirname, '..', name));
}

main();
