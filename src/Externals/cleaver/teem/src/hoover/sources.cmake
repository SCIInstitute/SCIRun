# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(HOOVER_SOURCES
  defaultsHoover.c
  hoover.h
  methodsHoover.c
  rays.c
  stub.c
  )

ADD_TEEM_LIBRARY(hoover ${HOOVER_SOURCES})
