# Superbuild External Caching

CI caches the compiled external libraries so that builds after the first only need to compile SCIRun itself — not Boost, Qt, Zlib, Python, and ~15 other dependencies from source.

## How it works

SCIRun uses a CMake [Superbuild](../Superbuild/): the outer CMake project drives `ExternalProject_Add` for each dependency, then builds SCIRun itself as the final step. Each external has a series of steps (download → update → configure → build → install), and CMake records completion in stamp files under `Externals/Stamp/`.

### What gets cached

The `actions/cache` step saves and restores two directories:

| Directory | Contents |
|-----------|----------|
| `bin/Externals/Install/` | Headers, compiled libs, and cmake config files for every external |
| `bin/Externals/Stamp/` | CMake stamp files that tell `make` which ExternalProject steps are done |

The cache key is `superbuild-v3-<OS>-<hash of Superbuild/*.cmake>-<variant>-py<bool>`. If any Superbuild cmake file changes, the hash changes and the cache is bypassed entirely — the next build is a full cold build that repopulates the cache.

### Why stamp-touching is needed

CMake rewrites the helper scripts in `Externals/tmp/` (e.g. `Zlib_external-cfgcmd.cmake`) on every configure run, giving them the current timestamp. The Makefile rules for ExternalProject steps depend on those scripts, so restored stamp files (which carry an older timestamp) always appear stale, causing `make` to try re-running every step against source directories that don't exist.

The fix is in `build.sh` (and an equivalent block in the Windows workflow): set the modification time of every file in `Externals/Stamp/` to the year **2100**, immediately after `cmake` configure but before `make`.

A plain `touch` (current time) is not sufficient. CMake's `ExternalProject_Add_Step` generates `add_custom_command` calls whose `DEPENDS` list includes the `ExternalProject.cmake` module file itself. If the GitHub Actions runner has a newer cmake installation than the runner that created the cached stamps, `ExternalProject.cmake` carries a newer timestamp and make considers every step stale regardless of touching. Setting stamps to 2100 makes them unconditionally newer than any cmake module file on any runner.

### Why `UPDATE_COMMAND ""`

By default, `ExternalProject_Add` with `GIT_REPOSITORY` generates a `*-gitupdate.cmake` script that fetches the remote and compares the HEAD hash to `GIT_TAG` on every build. With a restored stamp, this script would still run (for the same timestamp reason above), fail with `fatal: not a git repository` because `Source/` isn't cached, and abort the build.

Setting `UPDATE_COMMAND ""` in every git-based external tells CMake to generate a no-op update step instead — no `gitupdate.cmake` is written, no git operations are needed.

### Why `EP_UPDATE_DISCONNECTED TRUE`

Set globally in `Superbuild.cmake` as an extra safeguard. If for any reason an update stamp is missing (e.g. partial cache), CMake will run the (now no-op) update step once rather than triggering a network fetch.

### Cache key variants

Each OS × build variant × python flag gets its own cache entry. The restore-key is deliberately limited to same-variant matches (`-<variant>-` prefix but no python suffix) so that a `headless` cache is never used to satisfy a `gui` build — they compile different externals.

---

## When an external changes

### Bumping a `GIT_TAG`

Edit the relevant file in `Superbuild/` (e.g. `Superbuild/ZlibExternal.cmake`) and change `GIT_TAG`. Because the cache key hashes all `Superbuild/*.cmake` files, the existing cache is automatically invalidated. The next CI run does a full cold build (~1-2 hours) and saves a fresh cache.

**You do not need to bump the cache version number.**

### Adding a new external

1. Create `Superbuild/MyLibExternal.cmake` following the pattern of an existing file.
2. Add `UPDATE_COMMAND ""` alongside `GIT_TAG` (or after `URL` for tarball-based externals that don't need it).
3. Wire it into `Superbuild/Superbuild.cmake`.

The cache key hash will change automatically. Cold build on next CI run, new cache saved.

### Removing an external

Delete the cmake file and remove it from `Superbuild.cmake`. Cache key changes; next run is cold.

### Changing configure or build flags for an external

Edit the relevant `Superbuild/*.cmake` file. Cache key changes; next run is cold.

### Manually busting the cache

If the cache is known-bad (e.g. a broken partial save got promoted), bump the version prefix in `reusable-build.yml`:

```yaml
key: superbuild-v3-...   →   key: superbuild-v4-...
```

Update the `restore-keys` lines to match. On the next run every platform does a cold build and the old cache entries age out naturally (GitHub Actions evicts caches after 7 days of disuse).

### Local development note

The `touch Externals/Stamp/` step in `build.sh` runs on local builds too. If you change a `GIT_TAG` locally without clearing your build directory, `make` will skip the download/configure/build steps for that external and use the old installed version. The safest workflow when updating an external locally is:

```sh
rm -rf bin/Externals
./build.sh
```
