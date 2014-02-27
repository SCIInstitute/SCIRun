# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(GAGE_SOURCES
  ctx.c
  deconvolve.c
  defaultsGage.c
  filter.c
  gage.h
  kind.c
  miscGage.c
  print.c
  privateGage.h
  pvl.c
  scl.c
  sclanswer.c
  sclfilter.c
  sclprint.c
  shape.c
  st.c
  stack.c
  update.c
  vecGage.c
  vecprint.c
  )

ADD_TEEM_LIBRARY(gage ${GAGE_SOURCES})
