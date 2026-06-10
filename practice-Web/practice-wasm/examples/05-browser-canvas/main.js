import init, { fill_gradient } from "./pkg/browser_canvas_wasm_practice.js";

const canvas = document.querySelector("#canvas");
const output = document.querySelector("#output");
const context = canvas.getContext("2d");

try {
  await init();

  const width = canvas.width;
  const height = canvas.height;
  const pixels = fill_gradient(width, height);
  const image = new ImageData(new Uint8ClampedArray(pixels), width, height);

  context.putImageData(image, 0, 0);
  output.textContent = `Rendered ${width}x${height} RGBA pixels from WebAssembly.`;
} catch (error) {
  output.textContent = [
    "Could not load generated wasm-bindgen package.",
    "Build it first with: wasm-pack build --target web",
    "",
    String(error)
  ].join("\n");
}

