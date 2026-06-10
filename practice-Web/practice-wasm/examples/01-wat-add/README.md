# 01. WAT Add

This example introduces WebAssembly Text Format.

Build:

```console
wat2wasm add.wat -o add.wasm
```

Run in a browser:

```console
python -m http.server 8000
```

Then open `http://localhost:8000`.

Learning points:

- WAT is readable source for a WebAssembly module.
- The browser loads the compiled `.wasm`, not the `.wat`.
- Function signatures are validated before execution.

