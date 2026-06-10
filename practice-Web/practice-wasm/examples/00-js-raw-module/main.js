const bytes = new Uint8Array([
  0x00, 0x61, 0x73, 0x6d,
  0x01, 0x00, 0x00, 0x00,
  0x01, 0x07, 0x01, 0x60,
  0x02, 0x7f, 0x7f, 0x01,
  0x7f, 0x03, 0x02, 0x01,
  0x00, 0x07, 0x07, 0x01,
  0x03, 0x61, 0x64, 0x64,
  0x00, 0x00, 0x0a, 0x09,
  0x01, 0x07, 0x00, 0x20,
  0x00, 0x20, 0x01, 0x6a,
  0x0b
]);

const { instance, module } = await WebAssembly.instantiate(bytes);
const add = instance.exports.add;

const a = document.querySelector("#a");
const b = document.querySelector("#b");
const run = document.querySelector("#run");
const result = document.querySelector("#result");
const exportsView = document.querySelector("#exports");

exportsView.textContent = JSON.stringify(WebAssembly.Module.exports(module), null, 2);

function update() {
  const left = Number.parseInt(a.value, 10) || 0;
  const right = Number.parseInt(b.value, 10) || 0;
  result.value = `Result: ${add(left, right)}`;
}

run.addEventListener("click", update);
update();

