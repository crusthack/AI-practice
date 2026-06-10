const output = document.querySelector("#output");

try {
  const { instance } = await WebAssembly.instantiateStreaming(fetch("./memory.wasm"));
  const { memory, sum_i32 } = instance.exports;
  const values = new Int32Array(memory.buffer, 0, 5);

  values.set([3, 5, 8, 13, 21]);

  output.textContent = [
    `values: ${Array.from(values).join(", ")}`,
    `sum_i32(0, ${values.length}) = ${sum_i32(0, values.length)}`
  ].join("\n");
} catch (error) {
  output.textContent = [
    "Could not load memory.wasm.",
    "Build it first with: wat2wasm memory.wat -o memory.wasm",
    "",
    String(error)
  ].join("\n");
}

