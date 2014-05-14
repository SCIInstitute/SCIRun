# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(HEST_SOURCES
  defaultsHest.c
  hest.h
  methodsHest.c
  parseHest.c
  privateHest.h
  usage.c
  )

ADD_TEEM_LIBRARY(hest ${HEST_SOURCES})
