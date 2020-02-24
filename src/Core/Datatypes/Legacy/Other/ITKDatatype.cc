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
 *@file ITKDatatype.cc
 *
 *@author
 *   Darby J Brown
 *   School of Computing
 *   University of Utah
 *@date January 2003
 */

#include <Core/Datatypes/ITKDatatype.h>

#include <iostream>


using std::cerr;

namespace SCIRun {

static Persistent* make_ITKDatatype() {
  return new ITKDatatype;
}

PersistentTypeID ITKDatatype::type_id("ITKDatatype", "Datatype", make_ITKDatatype);

ITKDatatype::ITKDatatype() {
}

ITKDatatype::ITKDatatype(const ITKDatatype &copy) :
  fname(copy.fname)
{
  cerr << "*** ITKDatatype Copy Constructor needs to be FINISHED!\n";
}

ITKDatatype::~ITKDatatype() {
}


#define ITKIMAGE_VERSION 1

//////////
// PIO for ITKDatatype objects
void ITKDatatype::io(Piostream& stream) {
  /*  int version = */ stream.begin_class("ITKDatatype", ITKIMAGE_VERSION);
  /*
  if (stream.reading()) {
    Pio(stream, fname);
    //if (!(nrrdLoad(nrrd=nrrdNew(), strdup(fname.c_str())))) {
    //char *err = biffGet(NRRD);
    //cerr << "Error reading nrrd "<<fname<<": "<<err<<"\n";
    //free(err);
    //biffDone(NRRD);
    //return;
    //}
    fname="";
  } else { // writing
    if (fname == "") {   // if fname wasn't set up stream, just append .nrrd
      fname = stream.file_name + string(".itkimage");
    }
    Pio(stream, fname);
    //if (nrrdSave(strdup(fname.c_str()), nrrd, 0)) {
    //char *err = biffGet(NRRD);
    //cerr << "Error writing nrrd "<<fname<<": "<<err<<"\n";
    //free(err);
    //biffDone(NRRD);
    //return;
    //}
  }
  stream.end_class();
  */
}
}  // end namespace SCIRun
