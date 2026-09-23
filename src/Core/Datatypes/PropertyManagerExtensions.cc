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


#include <mutex>
#include <Core/Datatypes/PropertyManagerExtensions.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;

namespace
{
  // One FieldHandle commonly fans out to several modules that execute
  // concurrently, and each may be the first to touch its properties. Without
  // this lock both threads construct a PropertyManager, the loser's is freed
  // while the winner is still reading it, and the process dies in
  // copy_properties (#2700). A file-static lock keeps HasPropertyManager
  // trivially copyable-by-value, which MatrixBase relies on.
  std::mutex propertyManagerInitLock;
}

PropertyManager& HasPropertyManager::properties()
{
  // Same lazy init as the const overload; const_cast rather than duplicate it.
  return const_cast<PropertyManager&>(const_cast<const HasPropertyManager*>(this)->properties());
}

const PropertyManager& HasPropertyManager::properties() const
{
  std::lock_guard<std::mutex> guard(propertyManagerInitLock);

  if (!properties_)
    properties_.reset(new PropertyManager);

  return *properties_;
}

void SCIRun::Core::Datatypes::CopyProperties(const HasPropertyManager& from, HasPropertyManager& to)
{
  to.properties().copy_properties(&from.properties());
}
