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

These templates are gated on **two** in-flight branches (see the wave gate in
`../SKILL.md`); pinning them now guarantees a rewrite and a rebase collision:

- **`module-descriptors-metaprogram`** (C++20) drives the header/source templates:
  port declarations, the info macro (`MODULE_INFO_DEF` → 4-arg
  `MODULE_TRAITS_AND_INFO`), and the input/output helpers
  (`getRequiredInput_` / `sendOutput_`) — all being changed to C++20 variadic
  ports via string NTTPs.
- **`module-config-cleanup`** (#101) drives the config template: the `.module`
  JSON schema is unchanged, but it becomes the *sole* registration path once the
  hand-coded factory maps (`HardCodedAlgorithmFactory.cc`,
  `ModuleDialogFactory.cc`) are deleted — so `Module.module.tmpl` must emit a
  config that stands alone, with no companion factory-map edit.

Finalize each template **against its branch's final form once that branch
merges**, then flip the wave gate in `../SKILL.md`.

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
