# elefun

A complete native Windows game written in C11, inspired by the old butterfly catching tabletop game. A friendly elephant blows colorful butterflies through its tall trunk, and you move a net to scoop them up.

**[Download the Windows game](https://github.com/agammann/elefun/raw/refs/heads/main/Elefun.exe)** · **[Download the whole project](https://github.com/agammann/elefun/archive/refs/heads/main.zip)** · **[Verification notes](VERIFIED.md)**

![Elefun with a blue elephant, fluttering butterflies, and a green catching net](preview.png)

## Play

Download **Elefun.exe** and double click it, or use **Play.cmd** from the project ZIP. No installation, account, network connection, downloaded assets, or extra game framework is needed. The included executable is for 64 bit Windows 10 and Windows 11.

Select **SOLO**, **VS CPU**, or **2 PLAYERS**, then press **Enter** or click **LET THEM FLY**. A three second countdown gives everyone time to get ready. The elephant releases 31 butterflies, including one golden butterfly.

| Player | Move the net | Scoop |
| :--- | :--- | :--- |
| Player one | Move the mouse inside the meadow, or use W A S D | Left mouse button or Space |
| Player two | Arrow keys | Ctrl |

Tap to scoop once, or hold to keep scooping. Keyboard movement switches player one to keyboard control; moving the mouse inside the meadow switches back. Butterflies become catchable as they flutter downward. This digital adaptation gives them time to spread beyond the trunk. Fallen butterflies can also be collected by moving the net down to the ground.

Every butterfly is worth one point. Once the final airborne butterfly has landed, you have eight seconds to collect any on the ground. The round also ends if all butterflies are caught; a 65 second limit prevents a stalled round.

## Ways to play

| Choice | Behavior |
| :--- | :--- |
| Solo | Collect as many of the 31 butterflies as you can |
| Vs CPU | Race a computer controlled net |
| 2 players | Share one keyboard, or use mouse and keyboard together |
| Most butterflies | The highest score wins; equal scores tie |
| Golden butterfly | Catch the golden butterfly to win immediately; if nobody catches it, the highest score wins |
| Gentle, Breezy, Gusty | Change butterfly movement and computer net speed |

Modes, rules, and breeze settings can be changed in the lobby or after a round. They stay fixed during play.

| Key | Action |
| :--- | :--- |
| Enter | Start, replay, or resume |
| P | Pause or resume |
| R | Restart with a fresh countdown |
| Esc | Return to the lobby |
| M | Toggle sound |
| 1, 2, 3 | Select Solo, Vs CPU, or 2 players |
| G | Toggle the golden butterfly rule |
| B | Cycle breeze settings |

Losing window focus automatically pauses the game and releases held controls. The full frame is composed offscreen before being displayed, including after resizing, so the visible window is never cleared between animation frames. There are no full screen flashes or blinking victory effects.

## Build from source

Use LLVM MinGW or MinGW GCC on Windows. The game uses Windows system libraries: Win32, GDI, and WinMM. All artwork is drawn in code, and the sound effects are generated in memory.

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Compiler C:\path\to\clang.exe
```

With a suitable `clang` or `gcc` on PATH, run `build.ps1` directly. The script compiles the game with warnings treated as errors, builds the simulation tests, and runs them.

| File | Purpose |
| :--- | :--- |
| `src/game.c` and `src/game.h` | Portable C simulation, butterfly movement, nets, scoring, rules and opponents |
| `src/main.c` | Windows input, original artwork, complete frame presentation and sound |
| `tests/test_game.c` | 360 full simulated rounds and focused gameplay regression checks |
| `build.ps1` | Reproducible Windows build and simulation checks |
| `Play.cmd` | Simple launcher for the included executable |

## Additional checks

Run the hidden native integration suite with:

```powershell
Start-Process .\Elefun.exe -ArgumentList '--smoke-test' -WindowStyle Hidden -Wait
Get-Content .\smoke-result.txt
```

Failures include details in `smoke-details.txt`. This test exercises the actual window message handlers and presentation function without showing the window.

The renderer can export a BMP with `Elefun.exe --snapshot preview.bmp`. Append `playing`, `paused`, or `results` to inspect those layouts. These exports are rendering fixtures; simulation tests verify actual round completion.

![An active Elefun round with two nets and butterflies flying above the elephant](gameplay.png)

## Inspiration

This is an independent fan project, not an official Hasbro product. Elefun is a Hasbro trademark. The game uses original code, artwork and generated sounds; it does not include Hasbro artwork, logos, recordings or instruction sheets.

The tall blowing trunk, net catching, floor pickups and optional golden butterfly rule draw inspiration from [Hasbro's older Elefun instructions](https://www.hasbro.com/common/documents/dad261551c4311ddbd0b0800200c9a66/70ED7C3C5056900B1093BCA0F582B8F9.pdf). This adaptation uses digital net controls and shorter rounds.
