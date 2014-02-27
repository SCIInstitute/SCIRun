# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(PUSH_SOURCES
  action.c
  binning.c
  corePush.c
  defaultsPush.c
  forces.c
  methodsPush.c
  privatePush.h
  push.h
  setup.c
  )

ADD_TEEM_LIBRARY(push ${PUSH_SOURCES})
