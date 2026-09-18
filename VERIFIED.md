# Verification

For commands to reproduce these checks, see the [build and test guide](BUILD.md). For downloads and controls, see the [README](README.md).

Built locally with Clang 23.1.1, C11, optimization, and all enabled compiler warnings treated as errors.

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
| Frame presentation | Verified output pixels across four window sizes, eight frames per size |
| Graphics resources | GDI object count remained stable across 60 repeated draws |
| Layout inspection | Lobby, gameplay and results images inspected from the actual C renderer |
| Live window | Observed the lobby, pause, countdown, active flight with score updates, and a completed solo round showing 23 of 31 caught |
| Executable dependencies | Imports only Windows system libraries and the Windows Universal C Runtime |

The sound generator and sound toggle are implemented and compiled. Speaker output has not been verified by listening. Two people sharing a physical keyboard was not tested. Hardware key rollover can limit simultaneous keys on some keyboards.

The renderer composes both the scene and scaled window frame in memory and performs one complete transfer to the visible window. No intermediate dark or white background clearing is used.
