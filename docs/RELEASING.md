# Prepare a release

Builds and play need no Git. Release preparation uses a clean Git checkout plus the toolchain from [BUILD.md](../BUILD.md).

1. Update `ELEFUN_VERSION` and `ELEFUN_VERSION_NUMBERS` in `src/version.h`, README download paths and the `CHANGELOG.md` section together.
2. Commit the intended source changes. Do not commit generated executables, reports or ZIPs.
3. Run `build.ps1 -Compiler "C:\path\to\clang.exe"`, then `scripts\package-release.ps1`.
4. Run the consumer check below in a new output folder. It checks both ZIP hashes, every source Git blob, package identity, executable version, actual native controls/rounds, bad options and the newly extracted source build.
5. Review rendered results and the physical audio/two-player check in [PLAYTEST.md](PLAYTEST.md). Keep actual observations separate from automated checks.

```powershell
.\scripts\check-release.ps1 -Compiler "C:\path\to\clang.exe" -Out "C:\path\to\new-consumer-folder"
```

`release-artifacts/` contains exactly the source and Windows ZIPs, their individual checksum files and `SHA256SUMS`. Both ZIPs contain matching `RELEASE.json`: version, exact commit/tree, executable SHA256 and all source file Git blobs. Packaging refuses dirty source, changed build inputs, mismatched build/version, or existing artifacts. Use a new checkout for another package run. Source archive bytes are preserved even with `core.autocrlf=true`.

The Windows Actions job obtains the pinned compiler by checksum, builds and uses both deliverables, then uploads the checked five-file package set. On a successful **main push**, the publisher checks that main still equals the tested commit, checks the tag, uploads a draft release, verifies every asset digest and publishes last. Pull requests do not publish. An existing published version is left unchanged; use a new version for changed source.

Published ZIPs are unsigned Windows applications. Keep source and executable identity together when reporting or investigating a problem.
