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


#include <Core/Datatypes/Geometry.h>

using namespace SCIRun::Core;
using namespace Datatypes;

GeometryObject::GeometryObject(const GeometryIDGenerator& idGenerator, const std::string& tag) :
  objectName_(idGenerator.generateGeometryID(tag))
{
}

GeometryObject* GeometryObject::clone() const
{
  return nullptr; //TODO
}

void GeometryObject::addToList(GeometryBaseHandle handle, GeomList& list)
{
  if (handle.get() == this)
    list.insert(handle);
}

OsprayGeometryObject* OsprayGeometryObject::clone() const
{
  return nullptr; //TODO
}

CompositeOsprayGeometryObject::CompositeOsprayGeometryObject(const std::vector<OsprayGeometryObjectHandle>& objs) : objs_(objs)
{
  for (const auto& obj : objs_)
  {
    if (obj)
      box.extend(obj->box);
  }
}
