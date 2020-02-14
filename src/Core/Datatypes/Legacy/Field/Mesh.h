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


#ifndef CORE_DATATYPES_LEGACY_MESH_H
#define CORE_DATATYPES_LEGACY_MESH_H 1

#include <Core/Datatypes/Legacy/Base/Types.h>
#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Mesh/MeshTraits.h>
#include <Core/Datatypes/Legacy/Field/FieldFwd.h>
#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

class SCISHARE Mesh : public Core::Datatypes::Datatype, public Core::Datatypes::MeshTraits<VMesh>
{
public:
  Mesh();
  Mesh(const Mesh& copy);

  virtual ~Mesh();
  virtual Mesh *clone() const = 0;
  virtual MeshFacadeHandle getFacade() const = 0;

  /// These will become obsolete at some point
  enum
  {
    UNKNOWN		= 0,
    STRUCTURED		= 1 << 1,
    UNSTRUCTURED	= 1 << 2,
    REGULAR		= 1 << 3,
    IRREGULAR   	= 1 << 4
  };

  /// Synchronize system
  enum
  {
    NONE_E		= 0,
    NODES_E		= 1 << 0,
    EDGES_E		= 1 << 1,
    FACES_E		= 1 << 2,
    CELLS_E		= 1 << 3,
    ELEMS_E   = 1 << 4,
    DELEMS_E  = 1 << 5,
    ENODES_E   = 1 << 6,
    ALL_ELEMENTS_E      = NODES_E | EDGES_E | FACES_E | CELLS_E | ENODES_E | ELEMS_E | DELEMS_E,
    NORMALS_E		= 1 << 7,
    NODE_NEIGHBORS_E	= 1 << 8,
    ELEM_NEIGHBORS_E  = 1 << 9,
    NEIGHBORS_E = NODE_NEIGHBORS_E | ELEM_NEIGHBORS_E,
    NODE_LOCATE_E		= 1 << 10,
    ELEM_LOCATE_E		= 1 << 11,
    LOCATE_E		= NODE_LOCATE_E | ELEM_LOCATE_E,
    EPSILON_E = 1 << 12,
    BOUNDING_BOX_E = 1 << 12,
    FIND_CLOSEST_NODE_E		= 1 << 13,
    FIND_CLOSEST_ELEM_E		= 1 << 14,
    FIND_CLOSEST_E = FIND_CLOSEST_NODE_E | FIND_CLOSEST_ELEM_E
  };

  virtual bool synchronize(mask_type) { return false; }
  virtual bool unsynchronize(mask_type) { return false; }

  virtual int basis_order();

  /// Persistent I/O.
  void    io(Piostream &stream);
  static  PersistentTypeID type_id;
  static  const std::string type_name(int n = -1);
  virtual const TypeDescription *get_type_description() const = 0;

  /// Virtual interface functions:
  /// Get the virtual mesh interface, this returns the pointer to an internal
  /// object that has all the virtual functions. This object will be destroyed
  /// when the mesh is destroyed. The user does not need to destroy the VMesh.
  virtual VMesh* vmesh();
};

class SCISHARE MeshTypeID {
  public:
    // Constructor
    MeshTypeID(const std::string& type,
                MeshHandle (*mesh_maker)());
    MeshTypeID(const std::string& type,
                MeshHandle (*mesh_maker)(),
                MeshHandle (*latvol_maker)(size_type x, size_type y, size_type z, const Core::Geometry::Point& min, const Core::Geometry::Point& max)
                );
    MeshTypeID(const std::string& type,
                MeshHandle (*mesh_maker)(),
                MeshHandle (*image_maker)(size_type x, size_type y, const Core::Geometry::Point& min, const Core::Geometry::Point& max)
                );
    MeshTypeID(const std::string& type,
                MeshHandle (*mesh_maker)(),
                MeshHandle (*scanline_maker)(size_type x,const Core::Geometry::Point& min, const Core::Geometry::Point& max)
                );
    MeshTypeID(const std::string& type,
                MeshHandle (*mesh_maker)(),
                MeshHandle (*structhexvol_maker)(size_type x, size_type y, size_type z)
                );
    MeshTypeID(const std::string& type,
                MeshHandle (*mesh_maker)(),
                MeshHandle (*structquadsurf_maker)(size_type x, size_type y)
                );
    MeshTypeID(const std::string& type,
                MeshHandle (*mesh_maker)(),
                MeshHandle (*structcurve_maker)(size_type x)
                );


    std::string type;
    MeshHandle (*mesh_maker)();

    // Custom Constructors
    MeshHandle (*latvol_maker)(size_type x, size_type y, size_type z, const Core::Geometry::Point& min, const Core::Geometry::Point& max);
    MeshHandle (*image_maker)(size_type x, size_type y, const Core::Geometry::Point& min, const Core::Geometry::Point& max);
    MeshHandle (*scanline_maker)(size_type x, const Core::Geometry::Point& min, const Core::Geometry::Point& max);
    MeshHandle (*structhexvol_maker)(size_type x, size_type y, size_type z);
    MeshHandle (*structquadsurf_maker)(size_type x, size_type y);
    MeshHandle (*structcurve_maker)(size_type x);


};

SCISHARE MeshHandle CreateMesh(const std::string& type);
SCISHARE MeshHandle CreateMesh(const std::string& type, size_type x, size_type y, size_type z, const Core::Geometry::Point& min, const Core::Geometry::Point& max);
SCISHARE MeshHandle CreateMesh(const std::string& type, size_type x, size_type y, const Core::Geometry::Point& min, const Core::Geometry::Point& max);
SCISHARE MeshHandle CreateMesh(const std::string& type, size_type x, const Core::Geometry::Point& min, const Core::Geometry::Point& max);
SCISHARE MeshHandle CreateMesh(const std::string& type, size_type x, size_type y, size_type z);
SCISHARE MeshHandle CreateMesh(const std::string& type, size_type x, size_type y);
SCISHARE MeshHandle CreateMesh(const std::string& type, size_type x);

SCISHARE MeshHandle CreateMesh(FieldInformation &info);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,size_type x);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,size_type x,const Core::Geometry::Point& min,const Core::Geometry::Point& max);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,size_type x,size_type y);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,size_type x,size_type y,const Core::Geometry::Point& min,const Core::Geometry::Point& max);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,size_type x,size_type y,size_type z);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,size_type x,size_type y,size_type z,const Core::Geometry::Point& min,const Core::Geometry::Point& max);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,const std::vector<size_type>& x);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,const std::vector<size_type>& x,const Core::Geometry::Point& min,const Core::Geometry::Point& max);

SCISHARE MeshHandle CreateMesh(mesh_info_type mesh);
SCISHARE MeshHandle CreateMesh(mesh_info_type mesh,size_type x);
SCISHARE MeshHandle CreateMesh(mesh_info_type mesh,size_type x,const Core::Geometry::Point& min,const Core::Geometry::Point& max);
SCISHARE MeshHandle CreateMesh(mesh_info_type mesh,size_type x,size_type y);
SCISHARE MeshHandle CreateMesh(mesh_info_type mesh,size_type x,size_type y,const Core::Geometry::Point& min,const Core::Geometry::Point& max);
SCISHARE MeshHandle CreateMesh(mesh_info_type mesh,size_type x,size_type y,size_type z);
SCISHARE MeshHandle CreateMesh(mesh_info_type mesh,size_type x,size_type y,size_type z,const Core::Geometry::Point& min,const Core::Geometry::Point& max);
SCISHARE MeshHandle CreateMesh(mesh_info_type mesh, const std::vector<size_type>& x);
SCISHARE MeshHandle CreateMesh(mesh_info_type mesh, const std::vector<size_type>& x,const Core::Geometry::Point& min,const Core::Geometry::Point& max);

/// General case locate, search each elem.
template <class INDEX, class MESH>
bool elem_locate(INDEX &elem, MESH &msh, const Core::Geometry::Point &p)
{
  typename MESH::Elem::iterator iter, end;
  msh.begin(iter);
  msh.end(end);
  std::vector<double> coords(msh.dimensionality());
  while (iter != end) {
    if (msh.get_coords(coords, p, *iter))
    {
      elem = INDEX(*iter);
      return true;
    }
    ++iter;
  }
  return false;
}


} // end namespace SCIRun

#endif // Datatypes_Mesh_h
