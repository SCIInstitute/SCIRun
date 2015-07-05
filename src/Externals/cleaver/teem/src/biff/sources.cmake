# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(BIFF_SOURCES
  biff.h
  biffbiff.c
  )

ADD_TEEM_LIBRARY(biff ${BIFF_SOURCES})
