# Verification

For commands to reproduce these checks, see the [build and test guide](BUILD.md). For downloads and controls, see the [README](README.md).

Rebuilt and checked on Windows x64 on October 2, 2026 with Clang 23.1.2 from LLVM MinGW 20260922, C11, optimization, and all enabled compiler warnings treated as errors. The simulation, native message-handler checks, renderer fixtures, and executable imports below were rerun for this build.

| Check | Result |
| :--- | :--- |
| Full simulation suite | 360 rounds across all 3 modes, 2 rules, 3 breeze settings and 20 random seeds; all completed |
| Scoring | 9,589 catches across the full suite; every caught butterfly had exactly one owner and matching score |
| Butterfly accounting | All 31 butterflies conserved as waiting, airborne, grounded or caught in every checked step |
| Golden rule | Golden catch immediately ends the round and determines the winner |
| Floor pickups | Grounded butterflies are catchable for exactly one point |
| Simultaneous catches | Nearest net wins; exact overlaps alternate priority across butterfly IDs |
| Motion | Finite positions and velocities, playfield bounds and net bounds passed |
| State changes | Countdown, pause, resume, restart, idle completion and frozen final state passed |
| Native input | Both players movement, mouse movement, mouse scoops and short keyboard scoops passed |
| Native controls | Mode, rule, breeze, sound, start, focus loss and restart passed |
| Completed results | A seeded golden victory with a lower score stayed unchanged while selecting each mode, another rule, and another breeze; Enter, restart and lobby return applied the selected choices |
| Resized mouse input | At a 960 × 820 client size, letterbox clicks were ignored and the start button, mouse coordinates, net movement and scoop passed |
| Frame presentation | Verified output pixels across four window sizes, eight frames per size |
| Graphics resources | GDI object count remained stable across 60 repeated draws |
| Layout inspection | Lobby, gameplay, pause and results fixtures inspected from the actual C renderer, including the next-round settings label |
| Earlier live playtest | Previously observed the lobby, pause, countdown, active flight with score updates, and a completed solo round showing 23 of 31 caught; this manual playtest was not repeated for the current build |
| Executable dependencies | Imports only Windows system libraries and the Windows Universal C Runtime |

The completed-results regression failed before the settings fix and passed afterward. Native checks use a hidden Windows window and actual message handlers; the completed-round fixture is seeded, and rendering snapshots do not prove a live round was played.

The sound generator and sound toggle are implemented and compiled. Speaker output has not been verified by listening. Two people sharing a physical keyboard was not tested. Hardware key rollover can limit simultaneous keys on some keyboards.

The renderer composes both the scene and scaled window frame in memory and performs one complete transfer to the visible window. No intermediate dark or white background clearing is used.
