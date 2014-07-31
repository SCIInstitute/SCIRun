/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
/// @todo Documentation Core/Datatypes/Datatype.h 
#ifndef CORE_DATATYPES_DATATYPE_H
#define CORE_DATATYPES_DATATYPE_H 

#include <Core/Persistent/Persistent.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Datatypes/HasId.h>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  class SCISHARE Datatype : public Persistent, public HasIntegerId
  {
  public:
    Datatype();
    virtual ~Datatype();
    Datatype(const Datatype& other);
    Datatype& operator=(const Datatype& rhs);

    typedef HasIntegerId::id_type id_type;

    /// @todo
    template <typename T>
    const T* as() const
    {
      return dynamic_cast<const T*>(this);
    }

    virtual Datatype* clone() const = 0;
  };

}}}


#endif
