# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(LIMN_SOURCES
  cam.c
  defaultsLimn.c
  enumsLimn.c
  envmap.c
  hestLimn.c
  io.c
  light.c
  limn.h
  methodsLimn.c
  obj.c
  polydata.c
  polyfilter.c
  polymod.c
  polyshapes.c
  privateLimn.h
  qn.c
  renderLimn.c
  shapes.c
  splineEval.c
  splineMethods.c
  splineMisc.c
  transform.c
  )

ADD_TEEM_LIBRARY(limn ${LIMN_SOURCES})
