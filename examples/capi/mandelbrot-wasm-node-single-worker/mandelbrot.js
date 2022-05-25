const { performance } = require("perf_hooks");
const { createCanvas } = require("canvas");
const fs = require("fs");

const bytes = fs.readFileSync("mandelbrot.wasm");

const config = {
  x: -0.743644786,
  y: 0.1318252536,
  d: 0.00029336,
  iterations: 10000,
};

const memory = new WebAssembly.Memory({
  initial: 60,
  maximum: 60,
  shared: true,
});

WebAssembly.instantiate(bytes, {
  env: {
    memory: memory,
  },
}).then((Module) => {
  const start = performance.now();
  Module.instance.exports.mandelbrot(
    config.iterations,
    config.x,
    config.y,
    config.d
  );
  console.log("Elapsed Time:", performance.now() - start);
  const offset = Module.instance.exports.getImage();
  const canvasData = new Uint8Array(memory.buffer, offset, 1200 * 800 * 4);

  const canvas = createCanvas(1200, 800);
  const context = canvas.getContext("2d");
  const imageData = context.createImageData(1200, 800);
  imageData.data.set(canvasData);
  context.putImageData(imageData, 0, 0);

  const buffer = canvas.toBuffer("image/png");
  fs.writeFileSync("./output.png", buffer);
});
