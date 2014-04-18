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

#ifndef CORE_ALGORITHMS_FIELDS_RESAMPLEMESH_RESAMPLEREGULARMESH_H
#define CORE_ALGORITHMS_FIELDS_RESAMPLEMESH_RESAMPLEREGULARMESH_H 1

/// Datatypes that the algorithm uses
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>

/// Base class for algorithm
#include <Core/Algorithms/Util/AlgoBase.h>

/// for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE ResampleRegularMeshAlgo : public AlgoBase
{
  public:  
    /// Set defaults
    ResampleRegularMeshAlgo()
    { 
      /// Option for selecting the resamplign kernel
      add_option("method","box","box|tent|cubiccr|cubicrs|gaussian");
      add_scalar("sigma",1.0);
      add_scalar("extend",1.0);
      
      // resample none = keep as is, number= specify new number of samples
      //          factor = multiply number of samples by a factor 
      add_option("resamplex","none","none|number|factor");
      add_option("resampley","none","none|number|factor");
      add_option("resamplez","none","none|number|factor");

      add_scalar("xfactor",1.0);
      add_scalar("yfactor",1.0);
      add_scalar("zfactor",1.0);
      
      add_int("xnumber",128);
      add_int("ynumber",128);
      add_int("znumber",128);
    }
    
    /// Run the algorithm
    bool run(FieldHandle input, FieldHandle& output);
};

} // end namespace

#endif
