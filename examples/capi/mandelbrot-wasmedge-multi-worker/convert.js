const { createCanvas } = require("canvas");
const fs = require("fs");

let binary = fs.readFileSync("./output.bin");

const canvasData = new Uint8Array(binary, 0, 1200 * 800 * 4);

const canvas = createCanvas(1200, 800);
const context = canvas.getContext("2d");
const imageData = context.createImageData(1200, 800);
imageData.data.set(canvasData);
context.putImageData(imageData, 0, 0);

const buffer = canvas.toBuffer("image/png");
fs.writeFileSync("./output.png", buffer);
