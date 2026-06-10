const output = document.querySelector("#output");

try {
  const { instance } = await WebAssembly.instantiateStreaming(fetch("./add.wasm"));
  const { add, mul } = instance.exports;

  output.textContent = [
    `add(20, 22) = ${add(20, 22)}`,
    `mul(6, 7) = ${mul(6, 7)}`
  ].join("\n");
} catch (error) {
  output.textContent = [
    "Could not load add.wasm.",
    "Build it first with: wat2wasm add.wat -o add.wasm",
    "",
    String(error)
  ].join("\n");
}

