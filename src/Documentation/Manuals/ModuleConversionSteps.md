# How to convert a SCIRun 4 module to SCIRun 5

This is the concise checklist. For the full narrative with code templates, see
`docs/dev_doc/SCIRun5ModuleGeneration.md` (§"Converting Modules from SCIRun 4").
If you are Claude Code (or another agent), use the `convert-scirun-module` skill
in `.claude/skills/` — it carries the backlog manifest, the v4→v5 idiom
translation table, and the verification policy in machine-runnable form.

The guiding principle is the same as building a new module: **start minimal and
add one piece at a time, keeping the build green and committing after each step.**

## Where the backlog is

Unported v4 modules already live in `src/Modules/Legacy/<Domain>/` as `.cc`
files that are **commented out** in that directory's `CMakeLists.txt`
(e.g. `#PadRegularMesh.cc`). Uncommenting one and making it build + run is the
core of a conversion. A dormant file is recognizable by its v4 idioms:
`class Foo : public Module`, `virtual void execute();`, and `DECLARE_MAKER(Foo)`.

(Some commented entries have **no** `.cc` on disk — mostly `Teem/Tend` and the
Visualization `Show*Texture*` set. Those are from-scratch new modules, not
mechanical ports; treat them separately.)

## 0–2. Git setup

- Set up Git and GitHub, fork `https://github.com/SCIInstitute/SCIRun`, and add
  the upstream remote. See `docs/dev_doc/SCIRun5ModuleGeneration.md`
  §"Creating Your SCIRun Fork".
- Sync your fork, then create a branch off the latest `master` **per module**.
- Start from a clean tree (`git status`; `git stash`/`git reset` if needed).

## 3. Build SCIRun

Use the two-level Superbuild (see the repo `CLAUDE.md` and `docs/start/build.md`
for the authoritative, current instructions):

- **Requirements:** Qt 5.15 or 6.3.1+, a current CMake, and a C++17 compiler
  (C++20 on the `module-descriptors-metaprogram` branch). Do **not** use the old
  Qt 4.8 / CMake 2.8 toolchain referenced in ancient docs.
- **Linux/macOS:** `./build.sh` (add `--debug`, `--headless`, etc.).
- **Windows:** `.\build.ps1` (`-Debug`, `-Headless`, `-Jobs N`).
- Builds are out-of-source into `bin/`. First build downloads/compiles all
  externals (~15 GB, 30–90 min); incremental inner builds are fast: after the
  first success you can build just the inner project in `bin/SCIRun/`.
- Configure with `-DBUILD_TESTING=ON` so you can run unit + regression tests.

## 4. Create the module configuration file

Create `src/Modules/Factory/Config/<Module>.module`. Copy an existing one and
edit every line. It is JSON with three blocks — `module`, `algorithm`, `UI` —
each naming a `name`, `namespace`, and `header` (paths relative to `src/`). Set
the `algorithm` and `UI` blocks to `"N/A"` until those layers exist; fill the
`module` block first. `status`/`description` are free-text docs; everything else
becomes compiled code, so a typo is a build/link error.

## 5. Create the module header

Most v4 modules had no header — create `<Module>.h` next to the `.cc`. It
declares the class, its input/output ports, and the `execute()` /
`setStateDefaults()` methods, plus the module-info macro. Copy the structure
from a nearby **active** module in the same directory, or the template in
`src/Modules/Template/`. See `SCIRun5ModuleGeneration.md` §"Module Header File"
for the annotated layout.

## 6. Edit CMakeLists.txt

Uncomment the module's `.cc` in `src/Modules/Legacy/<Domain>/CMakeLists.txt` and
add the new `.h`. Depending on the layers you add you may touch up to five
CMakeLists (module, algorithm, UI, and their test dirs). Most dormant modules
are already in the right library and just need the line uncommented; if a new
library is needed, coordinate with @dcwhite.

## 7. Edit the module source so it compiles (empty `execute()`)

In the `.cc`: delete `DECLARE_MAKER`, the in-`.cc` class declaration, and every
`#include <Dataflow/Network/Ports/*Port.h>`. Fix the namespaces (at least the
domain namespace and `Networks`). Rewrite the constructor to
`Module(staticInfo_)` (append `,false` if there will be no UI). **Comment out the
body of `execute()`** and any helper functions for now — get an empty shell
compiling first. The v4→v5 idiom map is in the skill's
`reference/translation-table.md` and `SCIRun5ModuleGeneration.md`
§"Common Function Changes" / §"Common Build Errors".

## 8. Build green, confirm the module loads

Build. Once green, launch SCIRun and find the module in the list. Verify all
ports are present and correctly named (hover to check). Fix any port/name
mistakes now, before adding functionality.

## 9. Commit, sync.

Commit the building shell to your branch and sync.

## 10. Algorithm layer

If the compute is non-trivial, move it into
`src/Core/Algorithms/<Domain>/<Module>Algo.{h,cc}` (some v4 algorithms are
already copied under `Core/Algorithms/Legacy/` but not ported — reuse them).
The algorithm derives from `AlgorithmBase` and implements
`run(const AlgorithmInput&) const`. Wire the `algorithm` block in the `.module`
config and add the files to the algorithm CMakeLists. State-variable names are
declared once (in the algo header) and shared by module, dialog, and algo. See
`SCIRun5ModuleGeneration.md` §"Module Algorithm Code".

## 11. Commit, sync.

## 12. GUI layer

If the v4 module had a UI, add a dialog under
`src/Interface/Modules/<Domain>/`: copy the three files
(`<Module>Dialog.{h,cc}` + `<Module>Dialog.ui`) from a similar module or
`src/Interface/Modules/Template/`, edit the Qt `.ui` to match the needed widgets,
and hook the widgets to state variables. Fill the `UI` block in the `.module`
config and add the files to the interface CMakeLists. Implement
`setStateDefaults()` so the dialog shows sensible defaults. See
`SCIRun5ModuleGeneration.md` §"Module UI Code".

## 13. Restore `execute()` incrementally

Uncomment the compute one slice at a time, keeping the build green:
inputs (`getRequiredInput` / `getOptionalInput`) → compute or algo call →
outputs (`sendOutput` / `sendOutputFromAlgorithm`). Use the translation table for
each old idiom (`get_input_handle`, `send_output_handle`, `GuiVar` reads,
`.get_rep()`, dynamic-compilation removal, etc.). Commit frequently.

## 14. Legacy network import mapping

Add (or verify) this module's `<module name="...">` entry in
`src/Interface/Application/Resources/LegacyModuleImporter.xml`, mapping every old
v4 GUI/state variable name to its new state variable (with the right conversion
`<type>`). Without this, v4 networks that use the module import with missing or
wrong state. This is the fix surface tracked by issue #2532.

## 15. Add a test network + unit tests

- **Regression network:** the acceptance corpus is the v4 example networks in the
  SCIRunTestData repo (`Other/v4nets/`), run through the network-import
  regression tests. Find the network(s) that use your module and confirm they
  import **and** execute. If a network points to data that isn't present, note
  it. If **no** v4 network exercises your module, build a new test network from
  available data.
- **Unit tests:** copy `src/Modules/Template/ModuleUnitTest.cc` or
  `src/Core/Algorithms/Template/AlgorithmTestTemplate.cc` and test the algo/module
  on representative inputs. Compare against the v4 output where you can.

## 16. Document the module

Add the module markdown doc and symbolic link as described in
`SCIRun5ModuleGeneration.md` §"Documenting the New Module".

## 17. Pull request

Sync your branch with the latest `master`, push, and open a PR. Note which v4
network exercised the module and anything you had to flag (missing data,
hard-to-port paths). Address review feedback on the same branch.

Congratulations — you converted a module. Pick the next one from the backlog and
repeat.
