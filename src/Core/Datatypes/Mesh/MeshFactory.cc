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


/// @todo Documentation Core/Datatypes/Mesh/MeshFactory.cc

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

/// @todo Documentation Core/Datatypes/Mesh/MeshFactory.cc

#include <Core/Datatypes/Mesh/MeshFactory.h>
//#include <Core/Datatypes/Mesh/FieldInformation.h>
//#include <Core/Datatypes/Mesh/Mesh.h>
#include <Core/Datatypes/Mesh/LatticeVolumeMeshRegister.h>
#include <Core/Datatypes/Mesh/TriSurfMeshRegister.h>
#include <Core/GeometryPrimitives/Point.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;

MeshConstructionParameters::MeshConstructionParameters(size_type x, size_type y, size_type z, const Point& min, const Point& max)
  : x_(x), y_(y), z_(z), min_(min), max_(max) {}

CORE_SINGLETON_IMPLEMENTATION( MeshFactory )
CORE_SINGLETON_IMPLEMENTATION( MeshRegistry )

MeshFactory::MeshFactory() : registry_(MeshRegistry::Instance())
{
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  SCIRun::Core::Datatypes::registerLatticeVolumeMesh();
  SCIRun::Core::Datatypes::registerTriSurfMeshes();
  //fill in all mesh types here, via their basic register functions.
#endif
}

MeshRegistry::MeshRegistry()
{
}

/*
MeshHandle MeshFactory::CreateMesh(const FieldInformation& info, const MeshConstructionParameters& params)
{
  std::string type = info.get_mesh_type_id();
  return CreateMesh(type, params);
}

MeshHandle MeshFactory::CreateMesh(const FieldInformation& info)
{
  std::string type = info.get_mesh_type_id();
  return CreateMesh(type);
}
*/
MeshHandle MeshFactory::CreateMesh(const std::string& type)
{
  auto ctorInfo = registry_.meshTypeIdLookup_.findConstructorInfo(type);
  if (ctorInfo)
    return ctorInfo->defCtor_();
  return MeshHandle();
}

MeshHandle MeshFactory::CreateMesh(const std::string& type, const MeshConstructionParameters& params)
{
  auto ctorInfo = registry_.meshTypeIdLookup_.findConstructorInfo(type);
  if (ctorInfo)
    return ctorInfo->ctor_(params);
  return MeshHandle();
}

MeshRegistry::MeshTypeID::MeshTypeID() : defCtor_(0), ctor_(0)
{
}

MeshRegistry::MeshTypeID::MeshTypeID(const std::string& type, MeshDefaultConstructor defCtor, MeshConstructor ctor /* = 0 */) :
  type_(type), defCtor_(defCtor), ctor_(ctor)
{
  MeshRegistry::Instance().meshTypeIdLookup_.registerConstructorInfo(type, *this);
}

bool MeshRegistry::MeshTypeID::operator==(const MeshRegistry::MeshTypeID& other) const
{
  return defCtor_ == other.defCtor_ && ctor_ == other.ctor_;
}

bool MeshRegistry::MeshTypeID::operator!=(const MeshRegistry::MeshTypeID& other) const
{
  return !(*this == other);
}
