# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(DYE_SOURCES
  convertDye.c
  dye.h
  methodsDye.c
  )

ADD_TEEM_LIBRARY(dye ${DYE_SOURCES})
