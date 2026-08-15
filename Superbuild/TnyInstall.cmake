# install-tny.cmake
# Usage: cmake -Dsrc=... -Ddst=... -P install-tny.cmake

if(NOT DEFINED src OR NOT DEFINED dst)
  message(FATAL_ERROR "install-tny.cmake requires -Dsrc and -Ddst")
endif()

# Ensure the destination exists
file(MAKE_DIRECTORY "${dst}")

# Copy the *contents* of src into dst (note the trailing slash)
file(COPY "${src}/" DESTINATION "${dst}")