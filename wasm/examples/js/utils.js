const readFromBlobOrFile = (blob) =>
  new Promise((resolve, reject) => {
    const fileReader = new FileReader();
    fileReader.onload = () => {
      resolve(fileReader.result);
    };
    fileReader.onerror = ({
      target: {
        error: { code },
      },
    }) => {
      reject(Error(`File could not be read! Code=${code}`));
    };
    fileReader.readAsArrayBuffer(blob);
  });

const parseArgs = (Core, args) => {
  // args 最后一个参数后面要加上一个 null，所以用 args.length + 1
  const argsPtr = Core._malloc((args.length + 1) * Uint32Array.BYTES_PER_ELEMENT);
  args.forEach((s, idx) => {
    const buf = Core._malloc(s.length + 1);
    Core.writeAsciiToMemory(s, buf);
    Core.setValue(argsPtr + Uint32Array.BYTES_PER_ELEMENT * idx, buf, 'i32');
  });
  return [args.length, argsPtr];
};

// const melt = (Core, args) => {
//   Core.ccall(
//     'proxy_main',
//     'number',
//     ['number', 'number'],
//     parseArgs(Core, ['melt', '-nostdin', ...args]),
//   );
// };

// const runMelt = async (ifilename, data, args, ofilename, extraFiles = []) => {
//   let resolve = null;
//   let file = null;
//   const Core = await createMelt({
//     // printErr: (m) => {
//     //   console.log(m);
//     // },
//     // print: (m) => {
//     //   console.log(m);
//     //   if (m.startsWith('FFMPEG_END')) {
//     //     resolve();
//     //   }
//     // },
//   });
//   extraFiles.forEach(({ name, data: d }) => {
//     Core.FS.writeFile(name, d);
//   });
//   Core.FS.writeFile(ifilename, data);
//   melt(Core, args);
//   await new Promise((_resolve) => { resolve = _resolve });
//   if (typeof ofilename !== 'undefined') {
//     file = Core.FS.readFile(ofilename);
//     Core.FS.unlink(ofilename);
//   }
//   return { Core, file };
// };

const b64ToUint8Array = (str) => Uint8Array.from(atob(str), (c) => c.charCodeAt(0));

class MLTInstance {
  constructor(nativeInstance, melt) {
    this.nativeInstance = nativeInstance;
    this.melt = melt;
  }

  start() {
    return this.melt.mlt_instance_start(this.nativeInstance);
  }

  stop() {
    this.melt.mlt_instance_stop(this.nativeInstance);
  }

  release() {
    this.melt.mlt_instance_release(this.nativeInstance);
    this.nativeInstance = 0;
  }

  isStopped() {
    return this.melt.mlt_instance_is_stopped(this.nativeInstance) !== 0;
  }

  getError() {
    return this.melt.mlt_instance_get_error(this.nativeInstance);
  }

  waitForStop() {
    return new Promise((resolve) => {
      if (this.isStopped()) {
        resolve();
      }

      const intervalId = setInterval(() => {
        if (this.isStopped()) {
          clearInterval(intervalId);
          resolve();
        }
      }, 500);
    });
  }
}

let meltPromise;

function getMelt() {
  if (meltPromise) {
    return meltPromise;
  }
  meltPromise = createMelt({
    // printErr: (m) => {
    //   console.log(m);
    // },
    // print: (m) => {
    //   console.log(m);
    //   if (m.startsWith('FFMPEG_END')) {
    //     resolve();
    //   }
    // },
  }).then((mod) => {
    mod.MLTInstance = MLTInstance;
    MLTInstance.create = ({ args }) => {
      mod.mlt_instance_create();
      const nativeArgs = parseArgs(mod, ['melt', ...args]);
      const nativeInstance = mod.ccall('main', 'number', ['number', 'number'], nativeArgs);

      console.log('nativeInstance', nativeInstance);
      // 释放 argv
      mod._free(nativeArgs[1]);

      if (!nativeInstance) {
        return null;
      }

      return new MLTInstance(nativeInstance, mod);
    };

    return mod;
  });
  return meltPromise;
}

getMelt();
