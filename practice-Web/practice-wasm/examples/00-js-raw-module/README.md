# 00. JavaScript Raw Module

This example creates a tiny WebAssembly module directly from bytes.

It exports:

- `add(i32, i32) -> i32`

Open `index.html` in a browser and change the input values.

Learning points:

- A WebAssembly module can be instantiated from an `ArrayBuffer`.
- JavaScript is the host that calls exports.
- Numeric values cross the boundary directly.

