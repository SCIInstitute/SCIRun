/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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


// ServiceBase.h

#ifndef CORE_SERVICES_SERVICEBASE_H
#define CORE_SERVICES_SERVICEBASE_H 1

#include <Core/Services/share.h>

namespace SCIRun {

  class SCISHARE ServiceBase {
  public:
    enum {
      TAG_RQSV=1,   // REQUEST SERVICE
      TAG_RQSS,     // SERVICE REQUEST SUCCESS
      TAG_RQFL,     // SERVICE REQUEST FAILED
      TAG_AUTH,     // AUTHENTICATE
      TAG_AUFL,     // AUTHENTICATE FAILED
      TAG_AUSS,     // AUTHENTICATE SUCCESS
      TAG_STRT,     // STARTCOMMAND
      TAG_STFL,     // STARTCOMMAND FAILED
      TAG_STSS,     // STARTCOMMAND SUCCESS
      TAG_END_,     // END OF COMMUNICATION
      TAG_STDO,     // STDOUT OUTPUT
      TAG_STDE,     // STDERR OUTPUT
      TAG_EXIT,     // EXIT SIGNAL FROM SYSTEMCALL PROCESS EXITED
      TAG_END_STREAM // MARKER OF END OF COMMUNICATION
    };
  };

}
#endif

