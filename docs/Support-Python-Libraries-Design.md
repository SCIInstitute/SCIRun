Design doc: Support additional Python libraries (numpy, scipy)

Related issue: https://github.com/SCIInstitute/SCIRun/issues/2479

Overview

Goal: ensure SCIRun's Python wrappers and packaging work smoothly with common scientific Python libraries (numpy, scipy) so downstream users can interoperate with ndarrays, use numerical routines, and run tests/examples that depend on these libraries.

Motivation

Many users expect conversions between SCIRun data (Fields, Meshes, DenseMatrix) and numpy arrays without costly copies. CI and packaging must validate compatibility and reproducibility for supported numpy/scipy versions.

Requirements

Functional
- Zero-copy or documented conversion paths between SCIRun DenseMatrix/Field data and numpy ndarray where possible.
- Helper utilities in Python wrappers: to_numpy(), from_numpy(), memoryview access with clear ownership semantics.
- Example notebooks demonstrating workflows (convert, run numpy ops, convert back, visualize).

Build / Packaging
- CI jobs that install targeted numpy/scipy versions and run wrapper tests.
- Wheel/build rules documented for binary compatibility (manylinux, ABI tags) if shipping wheels.
- CMake option or wrapper build config to locate Python and numpy include/abi (find_package or pybind11 helpers).

Testing
- Pytest suite that covers conversions, roundtrip tests, and small numerical consistency checks across supported numpy versions.
- Tests run in CI matrix: at least two Python versions (e.g., 3.10, 3.11) and current numpy LTS + one older supported version.

API design
- Provide explicit conversion functions on Python objects:
  - DenseMatrix.to_numpy(copy=False) -> numpy.ndarray
  - DenseMatrix.from_numpy(ndarray, copy=False) -> DenseMatrix
  - Field/mesh extractors that return coordinate arrays and attribute arrays
- Clearly document ownership and lifetime: when copy=False, returned ndarray references internal memory until original object is GC'd or buffer is detached.
- Prefer pybind11 buffer protocol for native sharing where feasible.

Implementation Plan
1. Prototype conversion layer in a feature branch:
   - Implement to_numpy/from_numpy for DenseMatrix using pybind11 buffer interface.
   - Add helper functions for common types (float32/64) and shape conventions.
2. Add example Jupyter notebook and small integration test.
3. Update build scripts to detect numpy include/ABI for compiling wrappers; ensure pybind11 is used consistently.
4. Extend to Field/mesh conversions: provide arrays for points, connectivity, and per-vertex/per-cell attributes.
5. Add CI matrix entries (Python versions + numpy versions) and run pytest suite.

Acceptance criteria
- Conversions implemented and covered by tests.
- CI builds successfully with numpy present; tests pass in at least one CI job.
- Documentation and example notebook present in docs/python_examples/.

Risks and mitigations
- Binary compatibility: avoid shipping wheels initially; provide clear build-from-source instructions and CI verification.
- Ownership complexity: prefer explicit API (copy flag) and document lifetime rules. Use pybind11 buffer protocol to reduce errors.
- Large memory copies: benchmark common workflows and document performance expectations; add optional utilities for chunked conversion.

Open questions
- Which Python/numpy versions should be formally supported (LTS policy)?
- Is shipping binary wheels a near-term goal, or should initial effort focus on source builds and CI? 

Next steps
- Assign an owner to implement the DenseMatrix <-> numpy prototype.
- Create a feature branch and open a PR with the prototype and example notebook.
- Add CI job for one Python+numpy configuration and expand later.


Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>