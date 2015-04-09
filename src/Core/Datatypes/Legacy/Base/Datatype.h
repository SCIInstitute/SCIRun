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



///
///@file  Datatype.h
///@brief The Datatype Data type
///
///@author
///       David Weinstein
///       Department of Computer Science
///       University of Utah
///@date  July 1994
///
///

#ifndef CORE_DATATYPES_DATATYPE_H
#define CORE_DATATYPES_DATATYPE_H 1

#include <boost/type_traits.hpp>
#include <Core/Persistent/Persistent.h>
#include <Core/Thread/UsedWithLockingHandle.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Thread/RecursiveMutex.h>

#include <Core/Datatypes/share.h>

namespace SCIRun {

class SCISHARE Datatype : public Persistent, public UsedWithLockingHandle<RecursiveMutex>
{
public:
  /// unique id for each instance
  int generation;
  Datatype();
  Datatype(const Datatype&);
  Datatype& operator=(const Datatype&);
  virtual ~Datatype();

  virtual std::string dynamic_type_name() const = 0;

  static int compute_new_generation();
};

typedef LockingHandle<Datatype> DatatypeHandle;

#ifdef APPLE_LEOPARD_OR_SNOW_LEOPARD

#define SCI_DATATYPE_CAST sci_datatype_cast

#else

#define SCI_DATATYPE_CAST dynamic_cast

#endif

template <typename TPtr>
TPtr sci_datatype_cast(Datatype* handle)
{
  if (! handle)
    return 0;

  const std::string realType = handle->dynamic_type_name();

  typedef typename boost::remove_pointer<TPtr>::type return_type;
  const std::string returnType = return_type::type_id.type;

  if (Persistent::is_base_of(returnType, realType))
    return static_cast<TPtr>(handle);

  return 0;
}

} // End namespace SCIRun


#endif /* SCI_project_Datatype_h */
