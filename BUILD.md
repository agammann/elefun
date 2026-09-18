# Build and test Elefun

**You only need this guide to compile or test the source.** To play, use the executable linked in the [README](README.md#start-playing).

## Requirements

Use Windows 10 or 11 on x64, PowerShell, and an x64 Windows C compiler from **LLVM MinGW** or **MinGW GCC**. The compiler needs its Windows headers and libraries. An ordinary Clang installation without the MinGW toolchain is not sufficient for this build script.

The source uses C11 and the Windows Win32, GDI, and WinMM libraries. No external game engine or asset download is required. The published executable was built with Clang 23.1.1 from LLVM MinGW. MinGW GCC is supported by the script but has not been verified in this project.

To use the tested compiler, open the [LLVM MinGW 20260908 release](https://github.com/mstorsjo/llvm-mingw/releases/tag/20260908), download `llvm-mingw-20260908-ucrt-x86_64.zip` from its Assets list, and extract it. Keep the entire toolchain folder together. The compiler is `bin\clang.exe` inside that folder; you do not need to change PATH when using the explicit command below.

## Compile the game

1. Download and extract the [complete project ZIP](https://github.com/agammann/elefun/archive/refs/heads/main.zip), or clone this repository.
2. Open PowerShell in the extracted project folder, where `build.ps1` and `src` are located.
3. Run the command below, replacing the example compiler path with the path to your toolchain. Keep the quotes if the path contains spaces.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1 -Compiler "C:\path\to\llvm-mingw\bin\clang.exe"
```

If the appropriate `clang.exe` or `gcc.exe` is already on PATH, use:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1
```

The script also accepts the `CC` environment variable as a compiler path. An explicit `-Compiler` takes precedence; otherwise it checks `CC`, then searches PATH for `clang` before `gcc`. Pass `-Compiler` explicitly if you have several compilers installed.

The script compiles `Elefun.exe`, builds `build\test_game.exe`, and runs the simulation tests. A successful run prints two `PASS` lines followed by `Built Elefun.exe. Double click it to play.` Compiler warnings are treated as errors. Building replaces the included executable, so close any running copy first.

## Run simulation tests again

From the project folder after a successful build:

```powershell
& .\build\test_game.exe
if ($LASTEXITCODE -ne 0) { throw 'Simulation tests failed.' }
```

The suite covers 360 full rounds across all modes, rules, and breeze settings, plus scoring, ownership, gold catches, floor pickups, timing, and state changes. It does not open a game window.

## Check the native Windows application

From the project folder, run:

```powershell
$gamePath = Join-Path (Get-Location).Path 'Elefun.exe'
$check = Start-Process -FilePath $gamePath -ArgumentList '--smoke-test' -WorkingDirectory (Get-Location).Path -WindowStyle Hidden -Wait -PassThru
Get-Content .\smoke-result.txt
if ($check.ExitCode -ne 0) {
    Get-Content .\smoke-details.txt
    throw 'Native checks failed.'
}
```

Expect a `PASS` result and exit code 0. The hidden check exercises actual Windows message handlers for both players, the mouse, pause, focus changes, restart, settings, and sound toggling. It also checks rendered pixels at four window sizes and repeated drawing for GDI resource stability. It does not verify audible speaker output or replace playtesting with two people.

Each run writes `smoke-result.txt` and `smoke-details.txt` in the working directory. Both files and the `build` directory are ignored by Git.

## Export a rendering preview

From the project folder:

```powershell
Start-Process -FilePath .\Elefun.exe -ArgumentList '--snapshot preview.bmp' -WorkingDirectory (Get-Location).Path -WindowStyle Hidden -Wait
Start-Process -FilePath .\Elefun.exe -ArgumentList '--snapshot gameplay.bmp playing' -WorkingDirectory (Get-Location).Path -WindowStyle Hidden -Wait
```

The first command writes a lobby image; the second writes a gameplay image. You can replace `playing` with `paused` or `results`. These BMP files are rendering fixtures, not evidence that someone completed a live round. Generated BMPs are ignored by Git.

## Build troubleshooting

| Problem | Fix |
| :--- | :--- |
| Compiler not found | Use `-Compiler` with the full path to the toolchain's clang.exe or gcc.exe |
| Missing windows.h or Windows libraries | Use a complete LLVM MinGW or MinGW GCC toolchain and select its compiler explicitly |
| Permission denied while writing Elefun.exe | Close the running game and build in a folder you can write to |
| A command cannot find build.ps1 or Elefun.exe | Open PowerShell in the extracted project folder before running it |
| A test fails | Keep the full terminal output and, for native checks, both smoke text files when reporting the issue |

See [verification notes](VERIFIED.md) for tested behavior and remaining testing limits.
