/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

//    File   : ConvertToNrrd.h
//    Author : Martin Cole
//    Date   : Tue Jan  7 09:55:15 2003

#ifndef CORE_ALOGRITHMS_CONVERT_CONVERTTONRRD_H
#define CORE_ALOGRITHMS_CONVERT_CONVERTTONRRD_H 1

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/NrrdData.h>

#include <Core/Algorithms/Util/AlgoBase.h>

#include <Core/Algorithms/Converter/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

//! ConvertToNrrdBase supports the dynamically loadable algorithm concept.
//! when dynamically loaded the user will dynamically cast to a 
//! ConvertToNrrdBase from the DynamicAlgoBase they will have a pointer to.
class SCISHARE ConvertToNrrdAlgo : public AlgoBase
{
  public:
    ConvertToNrrdAlgo()
    {
      add_bool("build_points",true);
      add_bool("build_connections",true);
      add_bool("build_data",true);
      add_string("data_label","");
    }

    bool run(FieldHandle input, NrrdDataHandle& points, 
             NrrdDataHandle& connections,NrrdDataHandle& data);

};

} // end namespace

#endif // ConvertToNrrd_h
