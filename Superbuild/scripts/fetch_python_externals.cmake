# Fetches CPython's Windows build dependencies, with retries.
#
# PCbuild/get_externals.bat loops over the libraries without ever checking
# errorlevel, and get_external.py retries the extract but not the download. A
# 504 from github.com is therefore swallowed, and only surfaces minutes later as
# a MASM A1000 on a source file that was never extracted. Retrying here is cheap:
# get_externals.bat skips every library already on disk.
#
# Invoke with -DPCBUILD_DIR=<CPython source>/PCbuild.

if(NOT PCBUILD_DIR)
  message(FATAL_ERROR "PCBUILD_DIR must be set")
endif()

set(attempts 3)
set(fetched FALSE)

# cmd runs the .bat, and reads the "/" in a forward-slash path as a switch.
file(TO_NATIVE_PATH "${PCBUILD_DIR}/get_externals.bat" get_externals)

foreach(attempt RANGE 1 ${attempts})
  execute_process(
    COMMAND "${get_externals}"
    WORKING_DIRECTORY "${PCBUILD_DIR}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    ECHO_OUTPUT_VARIABLE
    ECHO_ERROR_VARIABLE
  )

  # get_external.py exits non-zero only on a failed extract; a failed download
  # escapes as an unhandled exception that the batch loop discards, so the
  # traceback is the only signal that anything went wrong.
  if(result EQUAL 0 AND NOT output MATCHES "Traceback \\(most recent call last\\)")
    set(fetched TRUE)
    break()
  endif()

  message(WARNING "Fetching CPython externals failed (attempt ${attempt} of ${attempts}).")
  if(attempt LESS attempts)
    math(EXPR backoff "${attempt} * 15")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep ${backoff})
  endif()
endforeach()

if(NOT fetched)
  message(FATAL_ERROR
    "Could not fetch CPython's external libraries after ${attempts} attempts. "
    "This is usually a transient github.com failure; rerun the build to resume.")
endif()
