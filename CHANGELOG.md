# Changelog

All notable changes to this project are documented in this file.

## Unreleased

### Changed
- Refactor: modernize CMake, Boost, Qt, and other external dependencies (PR #2463).
  - Superbuild CMake minimum: 3.20 (see Superbuild/CMakeLists.txt).
  - Boost and Qt updated via Superbuild; consult Superbuild configs for exact versions.

### Notes
- PR #2463 was developed and tested on Windows (Visual Studio 2022). Cross-platform verification pending.
- Address reported vector out-of-bounds and initialization issues before releasing.

(For full details see: https://github.com/SCIInstitute/SCIRun/pull/2463)
