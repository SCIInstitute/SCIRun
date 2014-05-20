# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(AIR_SOURCES
  754.c
  air.h
  array.c
  dio.c
  endianAir.c
  enum.c
  math.c
  miscAir.c
  mop.c
  parseAir.c
  privateAir.h
  randMT.c
  sane.c
  string.c
  threadAir.c
  )

ADD_TEEM_LIBRARY(air ${AIR_SOURCES})
