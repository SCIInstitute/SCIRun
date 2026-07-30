---
name: convert-scirun-module
description: Port a dormant SCIRun 4 module (commented-out .cc in src/Modules/Legacy/<Domain>/CMakeLists.txt) into a working SCIRun 5 module — config, header, source, optional algo/UI, LegacyModuleImporter.xml state mapping — and verify it against the v4 example-network import regression corpus. Use when asked to convert/port SCIRun 4 modules to v5, work the module-conversion backlog, or knock out entries from issue #1746.
---

# Convert a SCIRun 4 module to SCIRun 5

Machine-runnable playbook for porting one dormant v4 module per invocation. The
human-narrative version lives in `docs/dev_doc/SCIRun5ModuleGeneration.md`
(§"Converting Modules from SCIRun 4") and `src/Documentation/Manuals/ModuleConversionSteps.md`.

## ⛔ Wave gate — read first

The **file-generation templates are not final** until the C++20 boilerplate
branch `module-descriptors-metaprogram` merges (variadic ports via C++20 string
NTTPs, 4-arg `MODULE_TRAITS_AND_INFO` replacing `MODULE_INFO_DEF`,
`getRequiredInput_`/`sendOutput_`). Until then:

- **Do** use this skill to triage, scan idioms, plan a port, and regenerate the
  manifest.
- **Do not** start live conversions unless the user explicitly says to proceed
  against current-master macros (accepting a template redo at merge).
- The `templates/` dir is intentionally a Wave-2 stub — see `templates/README.md`.

When in doubt about whether the branch has merged: check
`git log --oneline master | grep -i "module-descriptors\|C++20"` or ask.

## Reference assets

| File | What it is |
|---|---|
| `reference/backlog_manifest.csv` | The worklist: every remaining module + domain, difficulty, LOC, has-algo/UI/config, cc path. Regenerate with `reference/gen_manifest.py`. |
| `reference/translation-table.md` | Frequency-ranked v4→v5 idiom map (mined from dormant-vs-active sources). The core of the "make it functional" step. |
| `reference/verification.md` | Acceptance ladder + self-certify-vs-flag policy, grounded in the v4-net import regression corpus. |
| `reference/idiom_scan.py` | Rescans dormant sources, reprints the idiom frequency table. |
| `templates/` | **Wave 2.** Fill-the-slot file templates (C++20-pending). |

## Picking the next module

Read `reference/backlog_manifest.csv`. Prefer `difficulty=easy`,
`cc_present=1`, and a domain with existing momentum (Fields, Math). Skip
`cc_present=0` rows — those are from-scratch, not ports (see verification.md).
Announce the chosen module and its stats before starting.

## The per-module loop

Work one module; keep the build green at every numbered step; commit after each.

1. **Locate the v4 source.** `cc_path` from the manifest. Read it fully. Note:
   ports (from `add_input_port`/`get_input_handle`), GuiVars (→ state), whether
   compute is inline (→ extract algo) or already in a `Core/Algorithms/Legacy`
   file (`has_legacy_algo=1`).
2. **Config.** Create `src/Modules/Factory/Config/<Module>.module`. Fill the
   `module` block first; set `algorithm`/`UI` to `"N/A"` until those layers exist.
3. **Header.** Create `<Module>.h` next to the `.cc` (v4 usually had none):
   namespaces, `INPUT_PORT`/`OUTPUT_PORT`, ctor decl, `execute()`,
   `setStateDefaults()`, info macro. *(Exact macro spellings: Wave-2 templates.)*
4. **Source de-v4.** In the `.cc`: delete `DECLARE_MAKER`, the in-`.cc` class
   decl, and all `Dataflow/Network/Ports/*Port.h` includes; fix namespaces;
   rewrite the ctor to `Module(staticInfo_[,false])`; **empty out `execute()`**
   (comment the body). Apply `translation-table.md` §1–2.
5. **Build shell.** Uncomment the `.cc` (and add the new `.h`) in the directory's
   `CMakeLists.txt`. Configure + build the inner target. Fix build errors
   (translation-table + SCIRun5ModuleGeneration.md §"Common Build Errors").
   Confirm the module loads and shows correct ports.
6. **Algo layer** (if compute is non-trivial). Move compute into
   `Core/Algorithms/<Domain>/<Module>Algo.{h,cc}` (or reuse the
   `Legacy` algo). Wire the `algorithm` block in the config. Apply
   translation-table §6.
7. **State + UI.** Implement `setStateDefaults()` (translation-table §3). If the
   v4 module had a GUI, add the dialog under `src/Interface/Modules/<Domain>/`
   and fill the `UI` config block. State var names go in `Parameters::`.
8. **Restore `execute()`** incrementally: inputs → compute/algo call → outputs
   (translation-table §2, §4, §5). Build green after each slice.
9. **Importer mapping.** Add/verify the module's `<module name="...">` entry in
   `src/Interface/Application/Resources/LegacyModuleImporter.xml`, mapping every
   old GuiVar name → new state var (verification.md).
10. **Verify** per the acceptance ladder in `verification.md`: find the
    referencing `v4nets` network(s), import + execute; self-certify or flag per
    the policy. No referencing net → build one from available data.
11. **Tests + docs.** Add a unit test (copy `src/Core/Algorithms/Template/AlgorithmTestTemplate.cc`
    or `src/Modules/Template/ModuleUnitTest.cc`) and a regression network; write
    the module markdown doc (SCIRun5ModuleGeneration.md §"Documenting the New Module").
12. **Report.** Summarize: gates passed, self-certified vs flagged, referencing
    network, anything left for the user. Append flags to `reference/flagged.md`.

## Batch / unattended mode

When told to "rip through" a set: iterate the loop over `difficulty=easy` rows,
committing per module, and **accumulate flags rather than stopping** on the
first hard one. Escalate immediately only for the hard-stop triggers in
verification.md (shared-infra changes, dynamic compilation, missing datatypes).
Produce a final table: done (self-certified) / flagged (reason) / skipped.
