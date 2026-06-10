# 04. C WASI CLI

This project introduces WebAssembly outside the browser through WASI.

Build with a WASI-capable Clang:

```console
clang --target=wasm32-wasi -O2 main.c -o hello.wasm
```

Run with a WASI runtime:

```console
wasmtime hello.wasm Ada Grace Linus
```

Learning points:

- WASI modules run outside the browser.
- The host provides capabilities such as arguments and file access.
- A WASI module still cannot access the whole operating system by default.

