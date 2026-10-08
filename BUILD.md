# Build and test Elefun

To play without compiling, download the Windows ZIP linked in [README.md](README.md#start-playing). These steps are for the matching v1.0.0 C source ZIP.

## Requirements

Use Windows 10 or 11 x64, Windows PowerShell 5.1 or newer, and the complete **LLVM MinGW 20260922 UCRT x86_64** toolchain. The checked compiler is **Clang 23.1.2**. An ordinary Clang install without Windows headers, libraries and `windres.exe` is insufficient. MinGW GCC may compile the C11 source, but this release is checked with the named LLVM MinGW toolchain.

Get `llvm-mingw-20260922-ucrt-x86_64.zip` from the [LLVM MinGW 20260922 release](https://github.com/mstorsjo/llvm-mingw/releases/tag/20260922) and extract the whole folder. Its SHA256 is `e3ad77d117a4bea19a7a3b333341824d79a5a371004a10e25b8504e7b3047666`. Keep `bin\clang.exe`, `bin\windres.exe`, headers and libraries together. Runtime uses only Windows system libraries and the Universal C Runtime supplied with Windows 10/11.

## Build from the extracted source

Extract `elefun_1.0.0_source.zip`, open PowerShell, then run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File "C:\path\to\elefun-1.0.0\build.ps1" -Compiler "C:\path\to\llvm-mingw-20260922-ucrt-x86_64\bin\clang.exe"
```

Use your own extracted folder paths. The script finds its source folder itself, so the terminal can be in another directory. If the tested compiler is on PATH, you can omit `-Compiler`; an explicit compiler takes precedence over `CC`, then PATH checks Clang before GCC.

The script builds in a new `build/check-...` folder, treats warnings as errors, runs simulation and native smoke checks, verifies file version 1.0.0, then promotes the checked executable to `build/Elefun.exe`. Close an already running source-built game before rebuilding. A compile or test failure keeps the previous successful build and leaves the failed check folder for diagnosis. Failed stages do not count as a successful new build.

Success prints the simulation PASS lines, native PASS line and `Built Elefun 1.0.0 at build/Elefun.exe. Run Play.cmd to play.` Open `Play.cmd` in the source folder. The `build/build.json` receipt records source input hashes and the resulting executable hash.

## Repeat the checks

From the source folder after building:

```powershell
& .\build\test_game.exe
if ($LASTEXITCODE -ne 0) { throw 'Simulation checks failed.' }
```

This covers **360 complete simulated rounds**, all three modes, both rules, all three breezes, 20 seeds, scoring/ownership, gold victory, floor pickups, ties, bounds, pause, invalid time steps, countdown and restart.

Run the actual native window checks in a writable folder:

```powershell
$game = (Resolve-Path .\build\Elefun.exe).Path
$check = Start-Process -FilePath $game -ArgumentList '--smoke-test' -WorkingDirectory (Get-Location).Path -WindowStyle Hidden -Wait -PassThru
Get-Content .\smoke-result.txt
Get-Content .\smoke-details.txt
if ($check.ExitCode -ne 0) { throw 'Native smoke checks failed.' }
```

The smoke check exercises actual input handlers, short keyboard/mouse scoops, pause/focus, settings/restart, frozen completed results, letterboxed mouse coordinates, four rendered sizes and stable GDI objects. Its completed-round state is seeded.

The longer check completes three natural rounds through the real Windows timer and normal input handlers, with no seeded scores or forced finish:

```powershell
$check = Start-Process -FilePath $game -ArgumentList '--seed 42 --acceptance-test' -WorkingDirectory (Get-Location).Path -WindowStyle Hidden -Wait -PassThru
Get-Content .\acceptance-result.txt
Get-Content .\acceptance-details.txt
if ($check.ExitCode -ne 0) { throw 'Native round checks failed.' }
```

Allow up to four minutes. It drives Solo, Vs CPU and 2 Players with automated Windows messages, checks pause/focus recovery and restart, and writes `acceptance-*.bmp` rendering images. Audio API results are recorded, but cannot prove that a person heard sound. Automated messages cannot prove physical keyboard rollover. See [the physical check](docs/PLAYTEST.md).

## Rendering fixtures and command errors

```powershell
Start-Process -FilePath $game -ArgumentList '--snapshot lobby.bmp lobby' -WindowStyle Hidden -Wait
Start-Process -FilePath $game -ArgumentList '--snapshot playing.bmp playing' -WindowStyle Hidden -Wait
```

The other phases are `paused` and `results`. These are seeded images from the real renderer, separate from naturally completed round images. `--version` prints `Elefun 1.0.0`; `--help` lists developer options. `--seed` accepts integers 1 through 4294967295. Unknown options, incompatible check modes and invalid seeds return a nonzero exit without opening a game. Snapshot output needs an existing writable folder.

## If a build fails

Select the complete named toolchain explicitly when Windows headers, libraries or `windres.exe` are missing. Build in a writable extracted folder and close `build/Elefun.exe` before promotion. Keep the full terminal output and the failed `build/check-...` folder. A native failure should include its result and details files. The game has no saved scores or persistent round state to migrate.

Packaging requires Git, but ordinary play and source builds do not. [RELEASING.md](docs/RELEASING.md) describes the exact-source consumer check and release workflow.
