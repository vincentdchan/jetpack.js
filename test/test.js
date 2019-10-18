const fs = require('fs');
const cp = require('child_process');
const { join } = require('path');

const cliName = 'zep-cli';

/**
 * 
 * @typedef {{
 *   cliPath: string,
 *   successCount: number,
 *   errorCount: number
 * }} TestContext
 */

/**
 * 
 * @param {fs.PathLike} path 
 * @returns {Promise<string[]>}
 */
async function readDirList(path) {
  const files = await fs.promises.readdir(path);
  return files.map(file => join(path, file));
}

/**
 * 
 * @param {string} filePath 
 * @param {TestContext} ctx
 */
async function testJSFile(filePath, ctx) {
  console.log('testing file: ' + filePath);

  const result = cp.spawnSync(ctx.cliPath, [`--entry=${filePath}`]);
  if (result.status == 0) {
    // console.log(result.stdout.toString('utf8'));
    ctx.successCount++;
  } else {
    console.error(result.stderr.toString('utf8'));
    ctx.errorCount++;
  }
}

/**
 * 
 * @param {fs.PathLike} path 
 * @param {TestContext} ctx
 */
async function traversePath(path, ctx) {
  const files = await readDirList(path);

  for (const filePath of files) {
    const stat = await fs.promises.stat(filePath);

    if (stat.isDirectory()) {
      await traversePath(filePath, ctx);
      continue;
    }

    if (stat.isFile() && filePath.endsWith('.js')) {
      await testJSFile(filePath, ctx);
    }
  }
}

/**
 * @returns {string}
 */
async function getCliPath() {
  const releasePath = join(__dirname, '..', 'cmake-build-release', cliName);
  const debugPath = join(__dirname, '..', 'cmake-build-debug', cliName);

  try {
    await fs.promises.stat(releasePath);
    return releasePath;
  } catch (err) {
    // release path error
  }

  try {
    await fs.promises.stat(debugPath);
    return debugPath;
  } catch (err) {
    throw new Error("Cli not found");
  }
}

/**
 * 
 * @param {TestContext} ctx 
 */
function printStatistic(ctx) {
  const { successCount, errorCount } = ctx;
  const totalCount = successCount + errorCount;
  console.log(`Test Pass: ${successCount}/${totalCount}`);
}

async function main() {
  const cliPath = await getCliPath();
  console.log('found cli: ', cliPath);

  const fixturePath = join(__dirname, './fixtures');

  /**
   * @type {TestContext}
   */
  const ctx = {
    cliPath,
    successCount: 0,
    errorCount: 0,
  }

  await traversePath(fixturePath, ctx);

  printStatistic(ctx)
}

main();
