/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

/// \author James Hughes
/// \date   March 2013

#ifndef SPIRE_APPSPECIFIC_SCIRUN_SRCOMMONATTRIBUTES_H
#define SPIRE_APPSPECIFIC_SCIRUN_SRCOMMONATTRIBUTES_H

#include <tuple>

#include "namespaces.h"
#include "spire/src/ShaderUniformStateManTemplates.h"

namespace CPM_SPIRE_SCIRUN_NS {

/// Common uniforms used by Spire.
/// \todo Look into the ability to rename these uniforms in the future. This is
/// so that Spire can adapt to other code bases.
class SRCommonAttributes
{
public:
  SRCommonAttributes()          {}
  virtual ~SRCommonAttributes() {}
  
  //----------------------------------------------------------------------------
  // Common transformations
  //----------------------------------------------------------------------------

  static std::tuple<const char*, spire::UNIFORM_TYPE> getObjectToWorldTrafo()
  {return std::make_pair("objToWorld", spire::UNIFORM_FLOAT_MAT4);}

};

} // namespace CPM_SPIRE_SCIRUN_NS 

#endif 
