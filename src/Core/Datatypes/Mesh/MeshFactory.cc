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

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#include <Core/Datatypes/Mesh/MeshFactory.h>
#include <Core/Datatypes/Mesh/FieldInformation.h>
#include <Core/Datatypes/Mesh/Mesh.h>
#include <Core/GeometryPrimitives/Point.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

MeshFactory::MeshFactory()
{
}

MeshFactory::~MeshFactory()
{
}

MeshHandle MeshFactory::CreateMesh(const FieldInformation& info, MeshTraits::size_type x, MeshTraits::size_type y, MeshTraits::size_type z, const Point& min, const Point& max)
{
  std::string type = info.get_mesh_type_id();
  return CreateMesh(type,x,y,z,min,max);
}

MeshHandle
  MeshFactory::CreateMesh(const std::string& type,
  MeshTraits::size_type x, MeshTraits::size_type y, MeshTraits::size_type z,
  const Point& min, const Point& max)
{
  auto it = MeshTypeIDTable->find(type);
  if (it != MeshTypeIDTable->end()) 
  {
    if ((*it).second->latvol_maker != 0)
    {
      return = (*it).second->latvol_maker(x,y,z,min,max);
    }
  }
  MeshTypeIDMutex->unlock();
  return (handle);
}