# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(MITE_SOURCES
  defaultsMite.c
  kindnot.c
  mite.h
  privateMite.h
  ray.c
  renderMite.c
  shade.c
  thread.c
  txf.c
  user.c
  )

ADD_TEEM_LIBRARY(mite ${MITE_SOURCES})
