# 03. Rust wasm-bindgen

This project is a minimal Rust-to-browser scaffold.

Prerequisites:

- Rust toolchain
- `wasm-pack`

Build:

```console
wasm-pack build --target web
python -m http.server 8000
```

Open `http://localhost:8000`.

Learning points:

- Rust exports are wrapped by generated JavaScript.
- `wasm-bindgen` makes browser interop ergonomic.
- The generated `pkg/` directory is a build artifact.

