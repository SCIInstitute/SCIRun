/*
** sanity.c: stand-alone demo of nrrdSanity() from Teem
** Copyright (C) 2008 Gordon Kindlmann
** License: WTFPL Version 2, December 2004
*/

#include <teem/nrrd.h>

int
main(int argc, char *argv[]) {
  int ret;
  char *me, *err;

  AIR_UNUSED(argc);
  me = argv[0];
  if (!nrrdSanity()) {
    err = biffGetDone(NRRD);
    fprintf(stderr, "%s: nrrdSanity failed:\n%s", me, err);
    free(err);
    ret = 1;
  } else {
    fprintf(stderr, "%s: nrrdSanity passed\n", me);
    ret = 0;
  }

  return ret;
}

/* EOF */
