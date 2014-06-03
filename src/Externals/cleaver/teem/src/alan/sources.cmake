# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(ALAN_SOURCES
  alan.h
  coreAlan.c
  enumsAlan.c
  methodsAlan.c
  )

ADD_TEEM_LIBRARY(alan ${ALAN_SOURCES})
