# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(MOSS_SOURCES
  defaultsMoss.c
  hestMoss.c
  methodsMoss.c
  moss.h
  privateMoss.h
  sampler.c
  xform.c
  )

ADD_TEEM_LIBRARY(moss ${MOSS_SOURCES})
