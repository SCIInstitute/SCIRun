# Changelog

All notable changes to this project are documented in this file.

## Unreleased

## 2026-07-03

### Changed
- Refactor: modernize CMake, Boost, Qt, and other external dependencies ([PR #2463](https://github.com/SCIInstitute/SCIRun/pull/2463)).
  - Superbuild CMake minimum: 3.20 (see `Superbuild/CMakeLists.txt`).
  - Boost and Qt updated via Superbuild; consult Superbuild configs for exact versions.
  - CI coverage expanded to include Intel Mac builds ([PR #2527](https://github.com/SCIInstitute/SCIRun/pull/2527)).
