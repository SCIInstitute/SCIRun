# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(BANE_SOURCES
  bane.h
  clip.c
  defaultsBane.c
  gkmsFlotsam.c
  gkmsHvol.c
  gkmsInfo.c
  gkmsMite.c
  gkmsOpac.c
  gkmsPvg.c
  gkmsScat.c
  gkmsTxf.c
  hvol.c
  inc.c
  measr.c
  methodsBane.c
  privateBane.h
  rangeBane.c
  scat.c
  trex.c
  trnsf.c
  valid.c
  )

ADD_TEEM_LIBRARY(bane ${BANE_SOURCES})
