# Regression networks

Every `.srn5` file under this directory is a CTest test. Read this before
adding one: the suite is easy to grow and easy to make slow, flaky or
meaningless.

## What a test actually checks

Each network runs as:

```
SCIRun -E <network.srn5> --no_splash --regression 60 -d <SCIRunTestData>
```

`-E` loads the network, executes it once, and quits when execution finishes.
`--regression 60` turns on regression mode (below). In GUI builds it also arms a
60-second timer; headless builds have no in-app timer, so a hang there is only
bounded by ctest's `--timeout`.

**A test passes when every module executes without calling `error()`, in time.**
The process exit code is the number of modules that errored.

| exit code | meaning |
|---|---|
| 0 | no module errored |
| N > 0 | N modules errored |
| 2 | GUI: timed out (also what two errored modules looks like; read the log) |
| 7 | GUI: network failed to load |

Nothing compares outputs. A network that renders garbage or computes the wrong
matrix still passes, unless the network checks its own result: a downstream
module that errors on a bad value, or a Python module calling `scirun_assert`.
If you want the test to catch wrong answers, build the check into the network.
Proper output verification is tracked in #2800.

## Regression mode

`--regression` changes app behaviour so runs are unattended and can run in
parallel:

- Most modal dialogs are suppressed. Not all: the "Disabled module" warning
  (a module compiled out of this build) and "Critical module error" still pop
  up and block until the timeout kills the test (#2795).
- QSettings are isolated per process (`SCIRun5_regression_<pid>`), so parallel
  tests don't race on the shared store. `scripts/run-regression-tests.sh` deletes
  these stores afterwards; plain `ctest` leaves them behind.
- Session tracing, save-before-execute and the deprecated-module prompt are off.
- On exit the process quits immediately, without normal teardown, so threads
  from streaming modules can't hang or crash it.

## How networks are picked up

`src/Testing/CMakeLists.txt` globs this directory **by folder**, gated on build
options:

| folder | included when |
|---|---|
| `Modules/`, `Network/`, `ImportExport/` | always |
| `Renderer/` | `WITH_GUI=ON` |
| `Python/` | `WITH_PYTHON=ON` (`Python/Optional/` never: it needs packages the bundled interpreter lacks) |
| `Ospray/` | `WITH_OSPRAY=ON` |
| `ExpectedError/` | never (not globbed) |

The test name is `.Test.ExampleNetwork.<basename>_srn`. **Only the basename is
used**, so two networks with the same filename in different folders collide.
Keep filenames unique across the whole tree (#2796).

Folder placement is how a network's dependencies are expressed. A network that
uses a module from an optional dependency must go in that dependency's folder.
Otherwise it runs in builds where the module is compiled out, and in GUI builds
it hangs on the "Disabled module" dialog until the timeout.

Any network whose file contains the text `ViewScene` (including `ViewSceneVtk`)
gets the CTest `RESOURCE_LOCK gpu`, so **those tests run one at a time**, even
under `ctest -j`. macOS serializes GPU work across contexts, and parallel
ViewScene networks were tripping the timeout. Each one you add adds its full
runtime to the serial part of the suite.

## Test data

Input files come from the separate
[SCIRunTestData](https://github.com/CIBC-Internal/SCIRunTestData) repo. Refer to
them as `%SCIRUNDATADIR%/...` in module state, never by absolute path.

The Superbuild fetches SCIRunTestData at `origin/master`, not a pinned commit.
Data you add there is visible to every branch's next build, and renaming or
deleting a file breaks every branch that still references it. Add files freely,
but don't move or remove them (#2799).

## Where it runs in CI

`.github/workflows/regression-tests.yml` runs nightly and on pushes to master,
**not on pull requests**:

| job | build | runs |
|---|---|---|
| `linux-headless-regression` | `WITH_GUI=OFF` | everything except `Renderer/` |
| `windows-headless-regression` | `WITH_GUI=OFF` | everything except `Renderer/` |
| `mac-gui-regression` | GUI, Python | everything including `Renderer/` |

The ctest-level timeout is 300 s per test, and the steps are currently
`continue-on-error` (#2711), so a failing network does not turn the run red yet.
No CI job runs with `WITH_OSPRAY` or `WITH_VTK`, so networks in those folders
only run on developer machines.

## Running locally

Configure with `-DBUILD_TESTING=ON -DSCIRUN_TEST_RESOURCE_DIR=/path/to/SCIRunTestData`, then:

```bash
scripts/run-regression-tests.sh bin/SCIRun -- -j3          # whole suite
scripts/run-regression-tests.sh bin/SCIRun -- -R myNetwork # one test
```

To see why a network fails, run its command line from `ctest -N -V -R <name>`
without `--regression`: you get the normal GUI, dialogs and module error text.

## Adding a network

1. Check that the network passes on its own with the command above, three
   times in a row. Intermittent passes become nightly noise.
2. Put it in the folder that matches its heaviest optional dependency, with a
   filename that is unique across this tree.
3. Keep it small and fast. The GUI timeout is 60 s under parallel load on a CI
   runner, which is a lot slower than your workstation. Use the smallest data
   that exercises the code.
4. Reference data only through `%SCIRUNDATADIR%`. If you need new data, add it
   to SCIRunTestData first.
5. Don't write output files into the source tree or the data repo.
6. If correctness matters, make the network fail on a wrong answer (see "What
   a test actually checks").
7. Re-run CMake: tests are globbed at configure time, so a new file is invisible
   to `ctest` until you do.

## Adding a batch of VTK networks

The suite has no VTK support yet. Before adding VTK networks:

- Create a `Vtk/` folder and an `IF(WITH_VTK)` glob for it in
  `src/Testing/CMakeLists.txt`, mirroring `Ospray/` (#2794). Don't put VTK
  networks in `Renderer/`: that folder runs in every GUI build, and builds
  without VTK compile `ViewSceneVtk` and `ShowFieldVtk` out.
- Every network containing `ViewSceneVtk` matches the `ViewScene` text check and
  runs serially. Many such networks will lengthen the suite linearly (#2797).
  Prefer fewer networks that each exercise several cases.
- No CI job will run them until one is configured with `WITH_VTK=ON` and
  `run-regression-tests: true` (#2798).
- A VTK network that renders the wrong image still passes. Image comparison is
  part of #2800.
