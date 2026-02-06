# install-sqlite-headers.cmake
# Usage: cmake -Dsrc=... -Ddst=... -P install-sqlite-headers.cmake

if(NOT DEFINED src OR NOT DEFINED dst)
  message(FATAL_ERROR "install-sqlite-headers.cmake requires -Dsrc and -Ddst")
endif()

file(MAKE_DIRECTORY "${dst}")
# Copy only the headers you care about; adjust if more headers are needed.
# If you want the whole dir content, use file(COPY ...) like the Tny script.
foreach(h sqlite3.h sqlite3ext.h)
  if(EXISTS "${src}/${h}")
    file(COPY "${src}/${h}" DESTINATION "${dst}")
  else()
    message(WARNING "Header not found: ${src}/${h}")
  endif()
endforeach()