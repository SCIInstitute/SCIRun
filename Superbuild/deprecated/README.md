# Deprecated Superbuild externals

These `*External.cmake` files are **not** part of the normal SCIRun build. None
are referenced by `Superbuild/Superbuild.cmake` (no `ADD_EXTERNAL`), and none are
managed by `Superbuild/VERSIONS.cmake`. They are kept here in case a special
build ever needs them again, rather than deleted outright.

| File | Why it's here |
| --- | --- |
| `LibPNGExternal.cmake` | Superseded by LodePNG; libpng was removed from the build in 2021 ("Remove LibPNG!"). |
| `GoogleTestExternal.cmake` | Superseded by the `googletest` git submodule (`src/Externals/submodules/googletest`). |
| `DataExternal.cmake` | Fetches Seg3D sample data (`CIBC-Internal/Seg3DData`); a Seg3D leftover, not a SCIRun code dependency. |
| `CtkExternal.cmake` | CTK is vendored as an in-tree code copy (`src/Interface/Modules/Base/CustomWidgets/CTK/`); this ExternalProject is disabled. |

## Re-enabling one

1. Move the file back into `Superbuild/`.
2. Add its source URL + pinned version to `Superbuild/VERSIONS.cmake` and update
   the file to consume those variables.
3. Add an `ADD_EXTERNAL( ${SUPERBUILD_DIR}/<File>.cmake <target> )` call in
   `Superbuild.cmake`.

See [`docs/dev_doc/DependencyUpdates.md`](../../docs/dev_doc/DependencyUpdates.md).
