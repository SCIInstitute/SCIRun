/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


/*
 *  NrrdToMatrix_IEPlugin.cc
 *
 *  Written by:
 *   Jeroen Stinstra
 *   Department of Computer Science
 *   University of Utah
 *
 */


// This is a plugin is meant to read simple text files with pure data
// in ascii format into a SCIRun matrix.

#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <string>
#include <teem/nrrd.h>

namespace SCIRun {

MatrixHandle NrrdToMatrix_reader(ProgressReporter *pr, const char *filename);

MatrixHandle NrrdToMatrix_reader(ProgressReporter *pr, const char *filename)
{
  MatrixHandle result = 0;

  Nrrd* nrrd = nrrdNew();

  if (nrrd)
  {
    if (nrrdLoad(nrrd, airStrdup(filename), 0))
    {
      char *err = biffGetDone(NRRD);
      pr->error("Read error on '" + std::string(filename) + "': " + err);
      free(err);
      return (result);
    }


    if (nrrd->dim > 2)
    {
      pr->error("Nrrd has a dimension larger than 2 which cannot be stored into a matrix");

      nrrdNix(nrrd);
      return (result);
    }

    if (nrrd->dim < 1)
    {
      pr->error("Nrrd dimension is zero");

      nrrdNix(nrrd);
      return (result);
    }

    int m = 1;
    int n = 1;
    if (nrrd->dim > 0) m = nrrd->axis[0].size;
    if (nrrd->dim > 1) n = nrrd->axis[1].size;

    result = new DenseMatrix(n,m);

    if (result.get_rep() == 0)
    {
      pr->error("Could not allocate the output Matrix");
      nrrdNix(nrrd);
      return (result);
    }
    double* data = result->get_data_pointer();

    int size = n*m;

    switch (nrrd->type)
    {
      case nrrdTypeChar:
      {
        char *ptr = reinterpret_cast<char *>(nrrd->data);
        for (int p=0; p<size; p++) { data[p] = static_cast<double>(ptr[p]); }
      }
      break;
      case nrrdTypeUChar:
      {
        unsigned char *ptr = reinterpret_cast<unsigned char *>(nrrd->data);
        for (int p=0; p<size; p++) { data[p] = static_cast<double>(ptr[p]); }
      }
      break;
      case nrrdTypeShort:
      {
        short *ptr = reinterpret_cast<short *>(nrrd->data);
        for (int p=0; p<size; p++) { data[p] = static_cast<double>(ptr[p]); }
      }
      break;
      case nrrdTypeUShort:
      {
        unsigned short *ptr = reinterpret_cast<unsigned short *>(nrrd->data);
        for (int p=0; p<size; p++) { data[p] = static_cast<double>(ptr[p]); }
      }
      break;
      case nrrdTypeInt:
      {
        int *ptr = reinterpret_cast<int *>(nrrd->data);
        for (int p=0; p<size; p++) { data[p] = static_cast<double>(ptr[p]); }
      }
      break;
      case nrrdTypeUInt:
      {
        unsigned int *ptr = reinterpret_cast<unsigned int *>(nrrd->data);
        for (int p=0; p<size; p++) { data[p] = static_cast<double>(ptr[p]); }
      }
      break;
      case nrrdTypeFloat:
      {
        float *ptr = reinterpret_cast<float *>(nrrd->data);
        for (int p=0; p<size; p++) { data[p] = static_cast<double>(ptr[p]); }
      }
      break;
      case nrrdTypeDouble:
      {
        double *ptr = reinterpret_cast<double *>(nrrd->data);
        for (int p=0; p<size; p++) { data[p] = static_cast<double>(ptr[p]); }
      }
      break;
      default:
      {
        pr->error("Unknown Nrrd type");
        nrrdNix(nrrd);
        result = 0;
        return (result);
      }
    }
    nrrdNix(nrrd);

  }

  return(result);
}

static MatrixIEPlugin NrrdToMatrix_plugin("NrrdFile","{.nhdr} {.nrrd}", "",NrrdToMatrix_reader,0);

} // end namespace
