# Verification

Elefun 1.0.0 keeps the existing 31-butterfly simulation and artwork. Checks here distinguish simulation, native Windows execution, rendering, and physical hardware observation.

The named local build uses Windows 11 x64 build 26300, Clang 23.1.2 / LLVM MinGW 20260922 UCRT x86_64, C11, optimization and compiler warnings as errors. Windows 10 x64 uses the same system API/UCRT target; a separate Windows 10 machine was not tested.

| Check | Scope |
| :--- | :--- |
| Simulation | 360 complete rounds across all 3 modes, 2 rules, 3 breezes and 20 seeds; 9,589 catches with owner/score conservation |
| Simulation edge behavior | Golden immediate victory, floor pickups, tie fairness, finite motion/bounds, pause, countdown, restart, idle completion, invalid time step and frozen results |
| Native smoke | Actual Win32 input handlers, both player movement/short scoops, mouse, pause/focus/restart, settings/sound toggle, completed results, letterboxed controls, four presentation sizes and stable graphics object count |
| Native round fixture | Real Windows timer and normal input handlers complete Solo, Vs CPU and 2 Players; no forced finish or seeded scores; pause/focus, result preservation and restart checks |
| Delivery | Matching source and Windows ZIPs carry version, exact commit/tree, executable hash and source Git blobs; the consumer script checks every source byte and builds the extracted source |
| Automation | Windows Actions repeats build, package and consumer checks; local outcomes and CI outcomes are distinct |

The native acceptance fixture sends controlled Windows messages and writes images from actual round states. It does not count as someone manually playing complete rounds. Generated rendering previews are seeded and are separate from naturally completed rounds.

Audio request results show whether Windows accepted playback calls. Physical sound and simultaneous key behavior require a separate observation using [PLAYTEST.md](docs/PLAYTEST.md); automated messages cannot establish either. Key rollover depends on the keyboard, and mouse-first player one controls reduce the number of simultaneous keys.

The game stores no persistent scores or settings and makes no network requests. Windows native rendering, input and sound are the supported delivery; Linux/macOS are not included executable targets.

The [October 2 verification record](docs/historical-verification-2026-10-02.md) preserves the earlier tests and their limits. Its live-play and hardware notes remain dated historical observations, not new v1 observations. Reproduce current checks with [BUILD.md](BUILD.md) and exact-package checks with [RELEASING.md](docs/RELEASING.md).
