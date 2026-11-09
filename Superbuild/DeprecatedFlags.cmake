# Shims for renamed or removed cache options. See #2668 for the naming rule
# (WITH_<dep> for optional dependencies, BUILD_<thing> for extra artifacts).
#
# Each shim is meant to live for one release and then be deleted, so keep the
# call sites in Superbuild.cmake together and dated.

# Old name still means what it meant: copy it forward, warn, keep going.
FUNCTION(scirun_renamed_option OLD NEW)
  IF(DEFINED ${OLD})
    IF(NOT DEFINED ${NEW})
      SET(${NEW} "${${OLD}}" CACHE BOOL "Set from deprecated ${OLD}" FORCE)
    ENDIF()
    MESSAGE(DEPRECATION "${OLD} was renamed to ${NEW}; using ${NEW}=${${NEW}}. "
      "Update your cmake line, scripts, or CMakeUserPresets.json.")
    UNSET(${OLD} CACHE)
  ENDIF()
ENDFUNCTION()

# Old name has no successor. Warns by default; pass FATAL when the option's
# sense changed, so a stale cache or CI line cannot silently produce the wrong build.
FUNCTION(scirun_removed_option OLD WHY)
  IF(DEFINED ${OLD})
    IF("FATAL" IN_LIST ARGN)
      MESSAGE(FATAL_ERROR "${OLD} is no longer a build option. ${WHY}")
    ENDIF()
    MESSAGE(DEPRECATION "${OLD} is no longer a build option and is ignored (was ${${OLD}}). ${WHY}")
    UNSET(${OLD} CACHE)
  ENDIF()
ENDFUNCTION()
