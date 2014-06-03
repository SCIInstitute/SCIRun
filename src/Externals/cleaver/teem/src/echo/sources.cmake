# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(ECHO_SOURCES
  bounds.c
  color.c
  echo.h
  enumsEcho.c
  intx.c
  lightEcho.c
  list.c
  matter.c
  methodsEcho.c
  model.c
  objmethods.c
  privateEcho.h
  renderEcho.c
  set.c
  sqd.c
  )

ADD_TEEM_LIBRARY(echo ${ECHO_SOURCES})
