
import loadAndRunJetpack from './jetpack-wasm';

const JSX_FLAG = 0x1
const CONSTANT_FOLDING_FLAG = 0x2
const MINIFY_FLAG = 0x100

export default function() {
  const myMoudle = {};
  loadAndRunJetpack(myMoudle);
  return myMoudle.ready.then(() => {
    return {
      minify(source) {
        const cSource = myMoudle.allocateUTF8(source);
        let result = 0;
        try {
          const flags = JSX_FLAG | CONSTANT_FOLDING_FLAG | MINIFY_FLAG;
          result = myMoudle._parse_and_codegen(cSource, flags);
          const errorCode = myMoudle.HEAP32[result >> 2];
          if (errorCode !== 0) {
            const msg = myMoudle.UTF8ToString(result + 4);
            throw new Error(msg);
          }
          return myMoudle.UTF8ToString(result + 4);
        } finally {
          myMoudle._free(cSource);
          if (result !== 0) {
            myMoudle._free(result);
          }
        }
      },

    };
  });
}
