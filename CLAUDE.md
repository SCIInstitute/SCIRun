# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

SCIRun uses a **two-level CMake Superbuild**. The outer Superbuild (in `Superbuild/`) downloads and compiles all external dependencies, then builds SCIRun itself (in `src/`). In-source builds are explicitly forbidden — always use a separate build directory (conventionally `bin/`).

### Configure & Build

**Windows (PowerShell):**
```powershell
.\build.ps1                        # Release build (default)
.\build.ps1 -Debug                 # Debug build
.\build.ps1 -Headless              # No Qt/GUI required
.\build.ps1 -WithTetgen            # Include Tetgen mesh generation
.\build.ps1 -Jobs 8                # Parallel jobs
.\build.ps1 -BuildDir "C:\MyBuild" # Custom build dir
```

**Linux/macOS:**
```bash
./build.sh                         # Release build
./build.sh --debug
./build.sh --with-tetgen
./build.sh -DBUILD_HEADLESS=ON
```

**Direct CMake:**
```bash
mkdir bin && cd bin
cmake ../Superbuild -G "Visual Studio 17 2022"  # Windows
cmake ../Superbuild                              # Linux/macOS
cmake --build . --config Release -j 8
```

Key CMake options (pass as `-DOPTION=VALUE` to the Superbuild):
- `BUILD_TESTING=ON` — enables unit and regression tests (off by default)
- `BUILD_WITH_PYTHON=ON` — Python API support (on by default)
- `BUILD_HEADLESS=ON` — skip Qt/GUI
- `WITH_TETGEN=ON` — Tetgen mesh library
- `WITH_OSPRAY=ON` — OSPRay ray-tracing renderer

First build requires ~15 GB disk space and 30–90 minutes.

### Running Tests

Tests require `BUILD_TESTING=ON` during CMake configuration. Run from the inner build directory (`bin/SCIRun/`):

```bash
cd bin/SCIRun
ctest --output-on-failure              # all tests
ctest -R "\.Test\." --output-on-failure          # unit tests only
ctest -R "ModuleName" --output-on-failure        # filter by name
```

**Regression tests** also require the SCIRunTestData repo:
```bash
# Clone test data:
git clone https://github.com/CIBC-Internal/SCIRunTestData

# Configure with test data path:
cmake ../Superbuild -DBUILD_TESTING=ON -DSCIRUN_TEST_RESOURCE_DIR=/path/to/SCIRunTestData

# Run regression tests:
scripts/run-regression-tests.sh bin/SCIRun -- -j3
scripts/run-regression-tests.sh bin/SCIRun -- -j3 -R Renderer
```

---

## Architecture

SCIRun is a **dataflow-based scientific visualization and computation platform**. Users connect modules (computational nodes) via ports into a network; execution flows through that graph. Network files use the `.srn5` extension.

### Major Subsystems

**`src/Core/`** — Foundation layer. Everything else depends on this.
- `Datatypes/` — Core data representations: `Field`, `Mesh`, `Matrix` (dense/sparse), `Geometry`, `Color`
- `GeometryPrimitives/` — 3D math: vectors, points, transforms
- `Algorithms/` — Algorithm implementations consumed by modules (separated from UI and dataflow concerns)
- `Dataflow/` and `src/Dataflow/` — Execution engine and module network graph
- `Python/` — Python interpreter integration and datatype conversion between Python and SCIRun types
- `Logging/` — Spdlog-backed logging

**`src/Dataflow/`** — The dataflow execution engine.
- `Engine/` — Scheduler and executor
- `Network/` — Module/port graph management
- `Serialization/` — `.srn5` network file format read/write
- `State/` — Module state management

**`src/Modules/`** — The computational units (modules) users connect in the GUI. Organized by domain:
- `Fields/`, `Math/`, `DataIO/`, `Visualization/`, `FiniteElements/`, `Forward/`, `Inverse/`, `BrainStimulator/`, `Python/`, `Render/`, `Basic/`
- Module logic is split: `Modules/` holds the dataflow glue; `Core/Algorithms/` holds the actual computation.

**`src/Interface/`** — Qt-based GUI layer.
- `Application/` — Main window and application shell
- `Modules/` — Per-module dialog implementations (mirrors `src/Modules/` hierarchy); `.ui` files alongside `.h/.cc`

**`src/Graphics/`** — OpenGL rendering and scene management.

**`src/Main/`** — Entry point (`scirunMain.cc`) and macOS bundle setup.

**`src/Externals/`** — Vendored third-party code (e.g., GoogleTest).

### Key Design Patterns

- **Module/Algorithm split**: Module classes (`src/Modules/`) handle dataflow ports and state; heavy computation lives in a corresponding `*Algo` class under `src/Core/Algorithms/`. This keeps algorithms testable independent of the network engine.
- **Factory-generated boilerplate**: Each module requires a config file in `src/Modules/Factory/Config/`. After editing these, regenerate CMake to get updated factory code.
- **Superbuild external dependencies**: Boost, Eigen, Qt (5.15 or 6.3.1+), GoogleTest, Teem, Zlib, Spdlog, GLEW, Qwt, and others are all fetched and built by the Superbuild; do not expect them pre-installed.

### Adding a New Module

See `docs/dev_doc/SCIRun5ModuleGeneration.md` for the full guide. Summary:
1. Add a `.module` config file in `src/Modules/Factory/Config/`
2. Implement `ModuleName.h/.cc` in the appropriate `src/Modules/<Domain>/` directory
3. Implement `ModuleNameAlgo.h/.cc` in the corresponding `src/Core/Algorithms/<Domain>/` directory
4. Add GUI dialog `ModuleNameDialog.h/.cc/.ui` under `src/Interface/Modules/<Domain>/` (if UI needed)
5. Re-run CMake to regenerate the factory

---

## Language & Standards

- **C++17** throughout (`CMAKE_CXX_STANDARD 17`)
- **Qt 5.15 or Qt 6.3.1+** for GUI components
- **Python 3.x** (when `BUILD_WITH_PYTHON=ON`)
- Unit tests use **GoogleTest/GoogleMock**

---

## Comments

Keep them terse. Two lines is usually enough; a comment longer than the code it explains is too long.

Say why the code is the way it is — the non-obvious constraint, the bug being worked around. Don't restate what the code does.

Background belongs in the commit message, not inline: investigation notes, verbatim error output, which commit broke what, how a bug was tracked down. `git log -S` and `git blame` will find it when someone needs it. Reference an issue or commit hash instead of retelling the story.
