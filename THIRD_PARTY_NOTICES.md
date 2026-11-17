# Third-party notices

The [MIT license](LICENSE) covers Elefun's original code, drawn artwork and generated sounds. Third-party trademarks remain with their owners.

The Windows executable links runtime code from the pinned LLVM MinGW 20260922 UCRT x86_64 toolchain. Its compiler is Clang 23.1.2. The toolchain's redistribution notices accompany both the source and Windows packages in `third-party/`.

- `LICENSE-llvm-mingw.txt` contains the toolchain's LLVM license and exceptions.
- `COPYING.MinGW-w64.txt` contains the MinGW-w64 copyright and license notices.
- `COPYING.MinGW-w64-runtime.txt` contains notices for the statically linked runtime.

These texts are copied unchanged from the [official pinned toolchain archive](https://github.com/mstorsjo/llvm-mingw/releases/tag/20260922). The compiler itself is not bundled. Windows system libraries and the Universal C Runtime are supplied by Windows. No commercial game artwork, recordings or fonts are distributed with Elefun.
