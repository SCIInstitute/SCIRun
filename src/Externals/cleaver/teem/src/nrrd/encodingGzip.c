/*
  Teem: Tools to process and visualize scientific data and images              
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "nrrd.h"
#include "privateNrrd.h"

typedef union {
  char **c;
  void **v;
} ptrHack;


int
_nrrdEncodingGzip_available(void) {

#if TEEM_ZLIB
  return AIR_TRUE;
#else
  return AIR_FALSE;
#endif
}

/*
** nio->byteSkip < 0 functionality contributed by Katharina Quintus
*/
int
_nrrdEncodingGzip_read(FILE *file, void *_data, size_t elNum,
                       Nrrd *nrrd, NrrdIoState *nio) {
  char me[]="_nrrdEncodingGzip_read", err[BIFF_STRLEN];
#if TEEM_ZLIB
  size_t sizeData, sizeRed, sizeChunk;
  int error;
  long int bi;
  unsigned int read;
  char *data;
  gzFile gzfin;
  ptrHack hack;

  sizeData = nrrdElementSize(nrrd)*elNum;
  /* Create the gzFile for reading in the gzipped data. */
  if ((gzfin = _nrrdGzOpen(file, "rb")) == Z_NULL) {
    /* there was a problem */
    sprintf(err, "%s: error opening gzFile", me);
    biffAdd(NRRD, err);
    return 1;
  }

  /* keeps track of how many bytes have been successfully read in */
  sizeRed = 0;
  
  /* zlib can only handle data sizes up to UINT_MAX ==> if there's more
     than UINT_MAX bytes to read in, we read in in chunks */
  sizeChunk = AIR_MIN(sizeData, UINT_MAX);
  
  if (nio->byteSkip < 0) { 
    /* We don't know the size of the size to skip before the data, so
       decompress the data first into a temporary memory buffer.  Then
       the byteskipping is then just memcpy-ing the appropriate region
       of memory from "buff" into the given "_data" pointer */
    char *buff;
    airArray *buffArr;
      
    /* setting the airArray increment to twice the chunk size means that for
       headers that are small compared to the data, the airArray never
       actually has to reallocate.  The unit is 1 because we are managing
       the reading in terms of bytes (sizeof(char)==1 by definition) */
    buff = NULL;
    hack.c = &buff;
    buffArr = airArrayNew(hack.v, NULL, 1, 2*sizeChunk);
    airArrayLenSet(buffArr, sizeChunk);
    if (!( buffArr && buffArr->data )) {
      sprintf(err, "%s: couldn't initialize airArray\n", me);
      biffAdd(NRRD, err);
      return 1;
    }
    
    /* we keep reading in chunks as long as there hasn't been an error,
       and we haven't hit EOF (EOF signified by read == 0).  Unlike the
       code below (for positive byteskip), we are obligated to read until
       the bitter end, and can't update sizeChunk to encompass only the 
       required data.  Cast on third arg ok because of AIR_MIN use above */
    while (!(error = _nrrdGzRead(gzfin, buff + sizeRed,
                                 AIR_CAST(unsigned int, sizeChunk),
                                 &read))
           && read > 0) {
      sizeRed += read;
      if (read >= sizeChunk) {
        /* we were able to read as much data as we requested, maybe there is
           more, so we need to make our temp buffer bigger */
        airArrayLenIncr(buffArr, sizeChunk);
        if (!buffArr->data) {
          sprintf(err, "%s: couldn't re-allocate data buffer", me);
          biffAdd(NRRD, err);
          return 1;
        }
      }
    }
    if (error) {
      sprintf(err, "%s: error reading from gzFile", me);
      biffAdd(NRRD, err);
      return 1;
    }
    if (sizeRed < sizeData + (-nio->byteSkip - 1)) {
      sprintf(err, "%s: expected " _AIR_SIZE_T_CNV " bytes and received only "
              _AIR_SIZE_T_CNV " bytes", me,
              AIR_CAST(size_t, sizeData + (-nio->byteSkip - 1)), sizeRed);
      biffAdd(NRRD, err);
      return 1;
    }
    /* also handles nio->byteSkip == -N-1 signifying extra N bytes at end */
    memcpy(_data, buff + sizeRed - sizeData - (-nio->byteSkip - 1), sizeData);
    airArrayNuke(buffArr);
  } else {
    /* no negative byteskip: after byteskipping, we can read directly
       into given data buffer */
    if (nio->byteSkip > 0) {
      for (bi=0; bi<nio->byteSkip; bi++) {
        unsigned char b;
        /* Check to see if a single byte was able to be read. */
        if (_nrrdGzRead(gzfin, &b, 1, &read) != 0 || read != 1) {
          sprintf(err, "%s: hit an error skipping byte %ld of %ld",
                  me, bi, nio->byteSkip);
          biffAdd(NRRD, err);
          return 1;
        }
      }
    }
    /* Pointer to chunks as we read them. */
    data = AIR_CAST(char *, _data);
    while (!(error = _nrrdGzRead(gzfin, data, sizeChunk, &read))
           && read > 0) {
      /* Increment the data pointer to the next available chunk. */
      data += read; 
      sizeRed += read;
      /* We only want to read as much data as we need, so we need to check
         to make sure that we don't request data that might be there but that
         we don't want.  This will reduce sizeChunk when we get to the last
         block (which may be smaller than sizeChunk). */
      if (sizeData >= sizeRed 
          && sizeData - sizeRed < sizeChunk) {
        sizeChunk = sizeData - sizeRed;
      }
    }
    if (error) {
      sprintf(err, "%s: error reading from gzFile", me);
      biffAdd(NRRD, err);
      return 1;
    }
    /* Check to see if we got out as much as we thought we should. */
    if (sizeRed != sizeData) {
      sprintf(err, "%s: expected " _AIR_SIZE_T_CNV " bytes and received "
              _AIR_SIZE_T_CNV " bytes",
              me, sizeData, sizeRed);
      biffAdd(NRRD, err);
      return 1;
    }
  }

  /* Close the gzFile.  Since _nrrdGzClose does not close the FILE* we
     will not encounter problems when dataFile is closed later. */
  if (_nrrdGzClose(gzfin)) {
    sprintf(err, "%s: error closing gzFile", me);
    biffAdd(NRRD, err);
    return 1;
  }
  
  return 0;
#else
  AIR_UNUSED(file);
  AIR_UNUSED(_data);
  AIR_UNUSED(elNum);
  AIR_UNUSED(nrrd);
  AIR_UNUSED(nio);
  sprintf(err, "%s: sorry, this nrrd not compiled with gzip enabled", me);
  biffAdd(NRRD, err); return 1;
#endif
}

int
_nrrdEncodingGzip_write(FILE *file, const void *_data, size_t elNum,
                        const Nrrd *nrrd, NrrdIoState *nio) {
  char me[]="_nrrdEncodingGzip_write", err[BIFF_STRLEN];
#if TEEM_ZLIB
  size_t sizeData, sizeWrit, sizeChunk;
  int fmt_i=0, error;
  char *data, fmt[4];
  gzFile gzfout;
  unsigned int wrote;
  
  sizeData = nrrdElementSize(nrrd)*elNum;

  /* Set format string based on the NrrdIoState parameters. */
  fmt[fmt_i++] = 'w';
  if (0 <= nio->zlibLevel && nio->zlibLevel <= 9)
    fmt[fmt_i++] = '0' + nio->zlibLevel;
  switch (nio->zlibStrategy) {
  case nrrdZlibStrategyHuffman:
    fmt[fmt_i++] = 'h';
    break;
  case nrrdZlibStrategyFiltered:
    fmt[fmt_i++] = 'f';
    break;
  case nrrdZlibStrategyDefault:
  default:
    break;
  }
  fmt[fmt_i] = 0;

  /* Create the gzFile for writing in the gzipped data. */
  if ((gzfout = _nrrdGzOpen(file, fmt)) == Z_NULL) {
    /* there was a problem */
    sprintf(err, "%s: error opening gzFile", me);
    biffAdd(NRRD, err);
    return 1;
  }

  /* zlib can only handle data sizes up to UINT_MAX ==> if there's more
     than UINT_MAX bytes to write out, we write out in chunks */
  sizeChunk = AIR_MIN(sizeData, UINT_MAX);

  /* keeps track of what how much has been successfully written */
  sizeWrit = 0;
  /* Pointer to the chunks as we write them. */
  data = (char *)_data;
  
  /* Ok, now we can begin writing. Cast on third arg ok because of
     AIR_MIN use above */
  while ((error = _nrrdGzWrite(gzfout, data,
                               AIR_CAST(unsigned int, sizeChunk),
                               &wrote)) == 0 
         && wrote > 0) {
    /* Increment the data pointer to the next available spot. */
    data += wrote;
    sizeWrit += wrote;
    /* We only want to write as much data as we need, so we need to check
       to make sure that we don't write more data than is there.  This
       will reduce sizeChunk when we get to the last block (which may
       be smaller than sizeChunk).
    */
    if (sizeData >= sizeWrit
        && (unsigned int)(sizeData - sizeWrit) < sizeChunk)
      sizeChunk = sizeData - sizeWrit;
  }
  
  if (error) {
    sprintf(err, "%s: error writing to gzFile", me);
    biffAdd(NRRD, err);
    return 1;
  }

  /* Check to see if we wrote out as much as we thought we should. */
  if (sizeWrit != sizeData) {
    sprintf(err, "%s: expected to write " _AIR_SIZE_T_CNV " bytes, but only "
            "wrote " _AIR_SIZE_T_CNV,
            me, sizeData, sizeWrit);
    biffAdd(NRRD, err);
    return 1;
  }
  
  /* Close the gzFile.  Since _nrrdGzClose does not close the FILE* we
     will not encounter problems when dataFile is closed later. */
  if (_nrrdGzClose(gzfout)) {
    sprintf(err, "%s: error closing gzFile", me);
    biffAdd(NRRD, err);
    return 1;
  }
  
  return 0;
#else
  AIR_UNUSED(file);
  AIR_UNUSED(_data);
  AIR_UNUSED(elNum);
  AIR_UNUSED(nrrd);
  AIR_UNUSED(nio);
  sprintf(err, "%s: sorry, this nrrd not compiled with zlib "
          "(needed for gzip) enabled", me);
  biffAdd(NRRD, err); return 1;
#endif
}

const NrrdEncoding
_nrrdEncodingGzip = {
  "gzip",      /* name */
  "raw.gz",    /* suffix */
  AIR_TRUE,    /* endianMatters */
  AIR_TRUE,   /* isCompression */
  _nrrdEncodingGzip_available,
  _nrrdEncodingGzip_read,
  _nrrdEncodingGzip_write
};

const NrrdEncoding *const
nrrdEncodingGzip = &_nrrdEncodingGzip;
