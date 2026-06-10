# 02. Memory From JavaScript

This example focuses on the host-memory boundary.

Build:

```console
wat2wasm memory.wat -o memory.wasm
python -m http.server 8000
```

Open `http://localhost:8000`.

Learning points:

- WebAssembly memory is an `ArrayBuffer`.
- JavaScript can write bytes into exported memory.
- WebAssembly functions usually receive offsets and lengths instead of objects.

