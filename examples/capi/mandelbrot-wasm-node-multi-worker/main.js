const { performance } = require("perf_hooks");
const { Worker } = require("worker_threads");
const { createCanvas } = require("canvas");
const fs = require("fs");

const memory = new WebAssembly.Memory({
  initial: 60,
  maximum: 60,
  shared: true,
});

const config = {
  x: -0.743644786,
  y: 0.1318252536,
  d: 0.00029336,
  iterations: 10000,
};

const num_threads = 4;
let finished = 0;
const start = performance.now();

for (let rank = 0; rank < num_threads; rank++) {
  const worker = new Worker("./worker.js", {
    workerData: { data: { memory, config, num_threads, rank } },
  });
  worker.on("message", (offset) => {
    finished++;
    if (finished === num_threads) {
      console.log("Elapsed Time:", performance.now() - start);
      const canvasData = new Uint8Array(memory.buffer, offset, 1200 * 800 * 4);

      const canvas = createCanvas(1200, 800);
      const context = canvas.getContext("2d");
      const imageData = context.createImageData(1200, 800);
      imageData.data.set(canvasData);
      context.putImageData(imageData, 0, 0);

      const buffer = canvas.toBuffer("image/png");
      fs.writeFileSync("./output.png", buffer);
    }
  });
}
