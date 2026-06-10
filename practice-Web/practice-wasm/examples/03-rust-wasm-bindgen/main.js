import init, { add, classify_score } from "./pkg/rust_wasm_bindgen_practice.js";

const output = document.querySelector("#output");

try {
  await init();
  output.textContent = [
    `add(11, 31) = ${add(11, 31)}`,
    `classify_score(88) = ${classify_score(88)}`
  ].join("\n");
} catch (error) {
  output.textContent = [
    "Could not load generated wasm-bindgen package.",
    "Build it first with: wasm-pack build --target web",
    "",
    String(error)
  ].join("\n");
}

