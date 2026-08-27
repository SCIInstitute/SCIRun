# Changelog

All notable changes to this project are documented in this file.

## Unreleased

### Changed
- Nightly release assets are now named for what they run on, not for the runner
  image that built them: `SCIRun-nightly-macos14-arm64-qt6-python.pkg` in place
  of `SCIRunMacInstaller.pkg`, `SCIRunMacNPInstaller.pkg`, and friends. The OS
  floor in the name is measured out of the packaged binary
  (`scripts/ci/build_info.py`), not assumed.
- The nightly release body is generated per run: a download table with each
  installer's OS floor, arch, Qt and Python, plus the commits since the previous
  nightly (`scripts/ci/nightly_release_notes.py`).

### Fixed
- The macOS GUI build for macOS 14 is now published. Nothing sets
  `CMAKE_OSX_DEPLOYMENT_TARGET`, so each job's binaries inherit their runner's
  minimum OS; every published mac installer was built on macOS 26 and therefore
  refused to launch on macOS 14 or 15. The macOS 14 job already existed and its
  installer was being discarded.
- The VS 2026 Windows build can no longer race its way onto a release. It is an
  `allow-failure` job that was not in the publisher's `needs` but did match the
  artifact glob.

## 2026-07-03

### Changed
- Refactor: modernize CMake, Boost, Qt, and other external dependencies ([PR #2463](https://github.com/SCIInstitute/SCIRun/pull/2463)).
  - Superbuild CMake minimum: 3.20 (see `Superbuild/CMakeLists.txt`).
  - Boost and Qt updated via Superbuild; consult Superbuild configs for exact versions.
  - CI coverage expanded to include Intel Mac builds ([PR #2527](https://github.com/SCIInstitute/SCIRun/pull/2527)).
