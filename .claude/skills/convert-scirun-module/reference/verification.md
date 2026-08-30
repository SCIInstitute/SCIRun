# Verification & escalation policy

The acceptance ground truth is the **v4 example-network corpus run through the
network-import regression tests** — not hand-waving that the code "looks ported."

## The corpus

- v4 networks live in the external **SCIRunTestData** repo under
  `Other/v4nets/` (resolved via `TestResources::rootDir()` /
  `SCIRUN_TEST_RESOURCE_DIR`). They are **not** in the SCIRun repo.
- Unit-level import is exercised by
  `src/Dataflow/Serialization/Network/Tests/LegacyNetworkFileImporterTests.cc`.
- The GUI-mode legacy-import regression suite runs the whole `v4nets` set
  (see `scripts/run-regression-tests.sh`; commits a06bdb3df / 95bd00fcd).
- v4 module names + **state variables** are mapped to v5 by
  `src/Interface/Application/Resources/LegacyModuleImporter.xml`. **Every port
  must add/verify its module's `<module name="...">` entry here**, or the v4
  network imports with missing/blank state. This is the concrete fix surface for
  issue #2532.

## Acceptance ladder for a newly ported module

Run these gates in order. Each gate must pass before the next.

1. **Builds.** The uncommented `.cc` (+ header, +algo/UI if added) compiles; the
   factory regenerates cleanly; the target links.
2. **Loads.** Module is registered and instantiates in a headless run without
   crashing; all ports present and correctly named. Check registration directly
   on the headless binary (no `BUILD_TESTING`, no Qt, no test data needed):

   ```bash
   SCIRun --list-modules | grep -i <ModuleName>
   ```

   `--list-modules` prints the registered module list and exits via the console
   path (`GlobalCommandBuilderFromCommandLine` → `PrintModulesCommand`); it is
   independent of `BUILD_TESTING`, which only gates the test targets. An empty
   grep result means the module did not register — recheck the `.module` config
   and the CMakeLists uncomment before going further.
3. **`LegacyModuleImporter.xml` entry** exists and maps every v4 state var this
   module used (cross-check against the dormant `.cc`'s old GuiVar names).
4. **v4 network imports.** Find the `v4nets` network(s) that reference this
   module; they import without an "undefined module" or lexical-cast error.
5. **v4 network executes** and produces output of the expected type without
   crashing.
6. **Numerical check** against golden data *if available* for that network.

## Self-certify vs flag (the bar Dan set)

| Situation | Action |
|---|---|
| Gates 1–5 pass **and** a `v4nets` network exists for the module | **Self-certify** the port (mechanically correct + exercised). Note in the PR which network exercised it. |
| A referencing network exists but **points to data that isn't present** (missing field/matrix files, missing external deps) | **Flag for Dan** — record module + network + the missing path. Do not fake data. Dan sorts these out. |
| **No** `v4nets` network references the module | **Build a new test network** from data that *is* available in SCIRunTestData / `src/Samples`, exercising the module's main path, and use it as the gate. If no suitable data exists, flag. |
| Gates 1–2 pass but 4–5 fail for a reason that needs judgment (see triggers below) | Take it to build+load, leave it **flagged, not done**. |

Never mark a module "done" on build-green alone. Numerical correctness beyond
"executes + right type" is Dan's sign-off unless golden data proves it.

## Hard-stop / escalation triggers

Stop the mechanical loop and flag with a one-line reason when you hit:

- A **dynamic-compilation** path (`get_compile_info` / `DynamicCompilation`) with
  no `VField`/`VMesh` equivalent.
- A **datatype the v5 tree lacks** (some Bundle/Nrrd/Tend corners).
- `SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED` / `SCIRUN4_CODE_TO_BE_CONVERTED_LATER`
  markers in the source (known-hard: issues #2530 / #2529).
- A **from-scratch** entry (no in-tree `.cc`: most `Teem/Tend`, Visualization
  `Show*Texture*`) — these are new-module work, not a port; flag unless Dan
  explicitly scopes them in.
- Any change that would touch **shared infrastructure** (engine, serialization,
  base classes) to make one module work — that's a design decision for Dan.

## Flag record format

When flagging, append one row to `reference/flagged.md` (create if absent):

```
- MODULE | domain | gate-reached | reason | referencing-v4net(s) | missing-data/notes
```

so a later pass (or Dan) can triage without re-deriving context.
