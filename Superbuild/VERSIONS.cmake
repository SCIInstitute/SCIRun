#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2026 Scientific Computing and Imaging Institute,
#  University of Utah.
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED... (see LICENSE)

# =============================================================================
# VERSIONS.cmake — central dependency manifest for the SCIRun Superbuild
# =============================================================================
#
# SINGLE SOURCE OF TRUTH for every external dependency's source URL and pinned
# version. Each *External.cmake file consumes the variables defined here instead
# of hard-coding its own repository and tag.
#
# Include this once, early in Superbuild.cmake (before any ADD_EXTERNAL call):
#
#     INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/VERSIONS.cmake)
#
# HOW TO UPDATE A DEPENDENCY
#   1. Change the *_GIT_TAG (or *_URL / *_URL_HASH) value below.
#   2. Prefer an immutable ref: a release tag (v1.2.3) or a full 40-char commit
#      SHA. Avoid branch refs like "origin/master" — they float and break
#      reproducible builds (see REPRODUCIBILITY note at the bottom).
#   3. Re-run the Superbuild from a clean build dir and smoke-test.
#   4. Note the change in docs/dev_doc/DependencyUpdates.md.
#
# Every value is a CACHE STRING so it can be overridden from the command line
# (-DZLIB_GIT_TAG=...) or a CI matrix without editing this file.

# Helper: declare a pinned dependency variable that is still user-overridable.
macro(sci_dep_version _var _default _doc)
  set(${_var} "${_default}" CACHE STRING "${_doc}")
  mark_as_advanced(${_var})
endmacro()

# -----------------------------------------------------------------------------
# Core numeric / math libraries
# -----------------------------------------------------------------------------
# Eigen — fetched as a release tarball (immutable, hashable). GOLD STANDARD pin.
sci_dep_version(EIGEN_VERSION  "3.4.0"  "Eigen release version")
sci_dep_version(EIGEN_URL
  "https://gitlab.com/libeigen/eigen/-/archive/${EIGEN_VERSION}/eigen-${EIGEN_VERSION}.tar.gz"
  "Eigen source tarball URL")
# TODO: add EIGEN_URL_HASH (SHA256) so downloads are integrity-checked.
sci_dep_version(EIGEN_URL_HASH "" "Eigen tarball SHA256 (URL_HASH SHA256=...)")

# -----------------------------------------------------------------------------
# I/O, compression, imaging
# -----------------------------------------------------------------------------
sci_dep_version(ZLIB_GIT_URL   "https://github.com/CIBC-Internal/zlib.git"    "zlib repository")
sci_dep_version(ZLIB_GIT_TAG   "scirun-pin-2026.07.27"     "zlib pin -> commit 713cffef (branch 1.3.1)")

sci_dep_version(LODEPNG_GIT_URL "https://github.com/CIBC-Internal/cibc-lodepng.git" "LodePNG repository")
sci_dep_version(LODEPNG_GIT_TAG "scirun-pin-2026.07.27"    "LodePNG pin -> commit 815f81a3 (branch master)")

sci_dep_version(FREETYPE_GIT_URL "https://github.com/CIBC-Internal/freetype.git" "FreeType repository")
sci_dep_version(FREETYPE_GIT_TAG "scirun-pin-2026.07.27"   "FreeType pin -> commit 8c48e9dc (branch scirun-5.0.0)")

sci_dep_version(TEEM_GIT_URL   "https://github.com/SCIInstitute/teem.git"     "Teem repository")
sci_dep_version(TEEM_GIT_TAG   "scirun-pin-2026.07.27"     "Teem pin -> commit f8ab306b (branch adjust-png)")

sci_dep_version(SQLITE_GIT_URL "https://github.com/CIBC-Internal/sqlite.git"  "SQLite repository")
sci_dep_version(SQLITE_GIT_TAG "scirun-pin-2026.07.27"     "SQLite pin -> commit 3944c9da (branch scirun-5.0.0-beta)")

# -----------------------------------------------------------------------------
# Rendering / graphics
# -----------------------------------------------------------------------------
sci_dep_version(GLM_GIT_URL    "https://github.com/g-truc/glm.git"            "GLM repository")
sci_dep_version(GLM_GIT_TAG    "0.9.9.8"                                      "GLM pinned tag")

sci_dep_version(GLEW_GIT_URL   "https://github.com/CIBC-Internal/glew.git"    "GLEW repository")
sci_dep_version(GLEW_GIT_TAG   "scirun-pin-2026.07.27"     "GLEW pin -> commit 2ef2e12f (branch scirun-5.0.0-beta)")

sci_dep_version(OSPRAY_GIT_URL "https://github.com/CIBC-Internal/ospray.git"  "OSPRay repository")
sci_dep_version(OSPRAY_GIT_TAG "scirun-pin-2026.07.27"     "OSPRay pin -> commit 174ba1aa (branch scirun-build-2.10)")

# -----------------------------------------------------------------------------
# GUI (Qt handled via find_package; Qwt built here)
# -----------------------------------------------------------------------------
sci_dep_version(QWT_GIT_URL         "https://github.com/CIBC-Internal/Qwt-cmake-wrapper.git" "Qwt cmake-wrapper repository")
sci_dep_version(QWT_WRAPPER_GIT_TAG "v0.1.1"                                  "Qwt wrapper pinned tag")
sci_dep_version(QWT_GIT_TAG         "v6.3.0"                                  "Qwt pinned tag")

# -----------------------------------------------------------------------------
# Language / scripting
# -----------------------------------------------------------------------------
# Python version is user-selectable; keep the default in sync with
# DEFAULT_PYTHON_VERSION in PythonExternal.cmake.
sci_dep_version(PYTHON_GIT_URL "https://github.com/CIBC-Internal/python.git"  "Python repository")
sci_dep_version(PYTHON_VERSION "3.13.1"                                       "Default Python version; built from CIBC-Internal/python origin/<version>")

sci_dep_version(BOOST_GIT_URL  "https://github.com/CIBC-Internal/boost.git"   "Boost repository")
sci_dep_version(BOOST_GIT_TAG  "v1.90.0"                                      "Boost pinned tag")

# -----------------------------------------------------------------------------
# Mesh / geometry
# -----------------------------------------------------------------------------
sci_dep_version(TETGEN_GIT_URL "https://github.com/CIBC-Internal/TetGen.git"  "TetGen repository")
sci_dep_version(TETGEN_GIT_TAG "v1.6.1"                                       "TetGen pinned tag")

sci_dep_version(CLEAVER2_GIT_URL "https://github.com/CIBC-Internal/Cleaver2Library.git" "Cleaver2 repository")
sci_dep_version(CLEAVER2_GIT_TAG "scirun-pin-2026.07.27"   "Cleaver2 pin -> commit 7e290469 (branch scirun-5.0.0-beta)")

# -----------------------------------------------------------------------------
# Logging / utility
# -----------------------------------------------------------------------------
sci_dep_version(SPDLOG_GIT_URL "https://github.com/gabime/spdlog"             "spdlog repository")
sci_dep_version(SPDLOG_GIT_TAG "v1.10.0"                                      "spdlog pinned tag")

sci_dep_version(TNY_GIT_URL    "https://github.com/CIBC-Internal/Tny.git"     "Tny repository")
sci_dep_version(TNY_GIT_TAG    "scirun-pin-2026.07.27"     "Tny pin -> commit 08cb2652 (branch scirun-5.0.0)")

# =============================================================================
# NOT MANIFEST-MANAGED
# =============================================================================
# These dependencies live in the tree but are intentionally NOT pinned here,
# because the Superbuild does not build them from these files:
#
#   - GoogleTest — vendored as a git submodule (src/Externals/submodules/
#     googletest, upstream google/googletest). Pinned by the submodule gitlink
#     SHA in the tree, which is already reproducible. The orphaned
#     Superbuild/deprecated/GoogleTestExternal.cmake is kept only for reference;
#     do not treat it as a live pin.
#   - libpng (Superbuild/deprecated/LibPNGExternal.cmake) — orphaned; SCIRun uses
#     LodePNG instead (libpng was removed from the build in 2021).
#   - Seg3DData (Superbuild/deprecated/DataExternal.cmake) — Seg3D leftover.
#   - CTK (Superbuild/deprecated/CtkExternal.cmake) — CTK is vendored as an
#     in-tree code copy (src/Interface/Modules/Base/CustomWidgets/CTK/); this
#     ExternalProject is disabled. Re-add its pin here if it is ever re-enabled.
#   - Qt — provided by the system / CI toolchain via find_package, not built by
#     the Superbuild (min version SCIRUN_QT_MIN_VERSION in Superbuild.cmake).
#   - SCIRunTestData / CIBCData — test fixtures fetched by TestDataConfig.cmake
#     and SCIRunDataExternal.cmake (the latter over SVN); data, not code deps.

# =============================================================================
# REPRODUCIBILITY NOTE
# =============================================================================
# Every GIT_TAG above is an immutable ref: either an upstream release tag
# (v1.2.3) or, for deps that track a SCIRun maintenance branch, an annotated
# `scirun-pin-<date>` tag that points at a fixed commit on that branch. Those
# tags were created 2026-07-27 at the then-current branch tips (the commit is
# recorded in each pin's doc string), so the build is byte-for-byte reproducible
# until a pin is deliberately bumped. A tag is used instead of a raw SHA purely
# for readability — it still resolves to one specific commit.
#
# When updating a dep that tracks a branch, create a NEW dated pin tag at the
# new tip and point the manifest at it (do not re-introduce a floating branch
# ref). For example, for zlib's branch "1.3.1":
#
#     sha=$(git ls-remote <repo-url> refs/heads/1.3.1 | cut -f1)
#     # create an annotated tag scirun-pin-<today> at $sha on the repo, then:
#     #   sci_dep_version(ZLIB_GIT_TAG "scirun-pin-<today>" "... commit <sha> ...")
#
# Remaining hardening (follow-up work):
#   - Add EIGEN_URL_HASH (and hashes for any future tarball deps) so downloads
#     are integrity-checked, not just version-pinned.
#
# The check-dependencies CI job (.github/workflows/dependency-check.yml) diffs
# these pins against upstream tags/releases and reports when newer versions are
# available (issue #2480, step 2).
