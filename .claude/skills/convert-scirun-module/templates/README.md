# Templates — WAVE 2 (deferred until C++20 boilerplate merges)

This directory will hold fill-the-slot generators for the per-module files:

- `Module.h.tmpl` — header: namespaces, port macros, ctor/execute/setStateDefaults
  decls, info macro.
- `Module.cc.tmpl` — source: includes, ctor, `execute()`, `setStateDefaults()`.
- `Module.module.tmpl` — factory config (module / algorithm / UI blocks).
- `ModuleAlgo.{h,cc}.tmpl` — algorithm layer.
- `ModuleDialog.{h,cc,ui}.tmpl` — Qt UI layer.
- `LegacyModuleImporter-entry.tmpl` — the `<module name="...">` state-map block.

## Why deferred

The exact spellings these templates must emit — port declarations, the info
macro (`MODULE_INFO_DEF` → 4-arg `MODULE_TRAITS_AND_INFO`), and the input/output
helpers (`getRequiredInput_` / `sendOutput_`) — are being changed by branch
`module-descriptors-metaprogram` (C++20 variadic ports via string NTTPs). Pinning
templates now guarantees a rewrite. Finalize them **against that branch's final
form once it merges**, then flip the Wave gate in `../SKILL.md`.

## Interim (if a port is needed before the merge)

Copy the nearest **active** module in the same `src/Modules/Legacy/<Domain>/`
directory as the live pattern (its `.h`/`.cc`, its `Factory/Config/*.module`, and
its dialog if any). The `reference/translation-table.md` rows tagged **[C++20]**
mark exactly the lines that will change form at merge time — keep them easy to
find so the interim ports can be swept forward.

Good current exemplars:
- No UI, no algo: `src/Modules/Examples/TestModuleSimple.{h,cc}`
- UI + algo: `src/Modules/Examples/SortMatrix.{h,cc}` + `src/Interface/Modules/Examples/SortMatrixDialog.*`
- Field port + algo, real module: `src/Modules/Legacy/Fields/GetFieldBoundary.cc`
