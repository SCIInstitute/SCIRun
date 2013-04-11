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


#ifndef CORE_DATATYPES_MESH_H
#define CORE_DATATYPES_MESH_H 1

#include <Core/Containers/StackVector.h>

#include <Core/Datatypes/Legacy/Base/Types.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>
#include <Core/Datatypes/Legacy/Field/FieldRNG.h>
#include <Core/Datatypes/Legacy/Field/FieldVIndex.h>
#include <Core/Datatypes/Legacy/Field/FieldVIterator.h>

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/BBox.h>

#include <Core/Utils/Legacy/Environment.h>
#include <Core/Utils/Legacy/Debug.h>

#include <Core/Datatypes/Datatype.h>

//! Incude needed for Windows: declares SCISHARE
#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

class Mesh;
class VMesh;
class Transform;
class TypeDescription;

typedef boost::shared_ptr<Mesh> MeshHandle;

// Maximum number of weights get_weights will return.
#define MESH_WEIGHT_MAXSIZE 64

// We reserve the last unsigned int value as a marker for bad mesh
// indices.  This is useful for example when there are no neighbors,
// or when elements are deleted.
#define MESH_INVALID_INDEX -1
#define MESH_NO_NEIGHBOR -1



class SCISHARE Mesh : public Core::Datatypes::Datatype
{
public: 

  typedef SCIRun::mask_type                  mask_type;  
  typedef SCIRun::index_type                 under_type;  
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::index_type                 size_type;
  typedef std::vector<index_type>            array_type;
  typedef std::vector<size_type>             dimension_type;


  //! Constructor
  Mesh();
  Mesh(const Mesh& copy) : Core::Datatypes::Datatype(copy) 
    { DEBUG_CONSTRUCTOR("Mesh");  }
  
  //! Destructor
  virtual ~Mesh();
  
  //! Clone the field needed by locking handle system
  virtual Mesh *clone() = 0;

  //! These will become obsolete at some point
  enum
  { 
    UNKNOWN		= 0,
    STRUCTURED		= 1 << 1,
    UNSTRUCTURED	= 1 << 2,
    REGULAR		= 1 << 3,
    IRREGULAR   	= 1 << 4
  };

  //! Synchronize system
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


  //! Optional virtual functions.
//  virtual bool has_normals() const { return false; }
//  virtual bool has_face_normals() const { return false; }
//  virtual bool is_editable() const { return false; } // supports add_elem(...)
//  virtual int  dimensionality() const = 0;
//  virtual int  topology_geometry() const = 0;
//  virtual bool get_dim(vector<size_type>&) const { return false; }
//  virtual bool get_search_grid_info(size_type& /*i*/, size_type& /*j*/, 
//                          size_type& /*k*/, Transform& /*trans*/) { return false; }
  virtual int basis_order();
  
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  //! Persistent I/O.
  void    io(Piostream &stream);
  static  PersistentTypeID type_id;
  static  const std::string type_name(int n = -1);
  virtual const TypeDescription *get_type_description() const = 0;
#endif

  // The minimum value for elemental checking
//  double MIN_ELEMENT_VAL;
  
  //! Virtual interface functions:
  //! Get the virtual mesh interface, this returns the pointer to an internal
  //! object that has all the virtual functions. This object will be destroyed
  //! when the mesh is destroyed. The user does not need to destroy the VMesh.
  virtual VMesh* vmesh();
};

class SCISHARE MeshTypeID {
  public:
    // Constructor
    MeshTypeID(const std::string& type, 
                MeshHandle (*mesh_maker)());
    MeshTypeID(const std::string& type, 
                MeshHandle (*mesh_maker)(),
                MeshHandle (*latvol_maker)(Mesh::size_type x, Mesh::size_type y, Mesh::size_type z, const Core::Geometry::Point& min, const Core::Geometry::Point& max)
                );
    MeshTypeID(const std::string& type, 
                MeshHandle (*mesh_maker)(),
                MeshHandle (*image_maker)(Mesh::size_type x, Mesh::size_type y, const Core::Geometry::Point& min, const Core::Geometry::Point& max)
                );
    MeshTypeID(const std::string& type, 
                MeshHandle (*mesh_maker)(),
                MeshHandle (*scanline_maker)(Mesh::size_type x,const Core::Geometry::Point& min, const Core::Geometry::Point& max)
                );
    MeshTypeID(const std::string& type, 
                MeshHandle (*mesh_maker)(),
                MeshHandle (*structhexvol_maker)(Mesh::size_type x, Mesh::size_type y, Mesh::size_type z)
                );
    MeshTypeID(const std::string& type, 
                MeshHandle (*mesh_maker)(),
                MeshHandle (*structquadsurf_maker)(Mesh::size_type x, Mesh::size_type y)
                );
    MeshTypeID(const std::string& type, 
                MeshHandle (*mesh_maker)(),
                MeshHandle (*structcurve_maker)(Mesh::size_type x)
                );

    
    std::string type;
    MeshHandle (*mesh_maker)();
    
    // Custom Constructors
    MeshHandle (*latvol_maker)(Mesh::size_type x, Mesh::size_type y, Mesh::size_type z, const Core::Geometry::Point& min, const Core::Geometry::Point& max);
    MeshHandle (*image_maker)(Mesh::size_type x, Mesh::size_type y, const Core::Geometry::Point& min, const Core::Geometry::Point& max);
    MeshHandle (*scanline_maker)(Mesh::size_type x, const Core::Geometry::Point& min, const Core::Geometry::Point& max);
    MeshHandle (*structhexvol_maker)(Mesh::size_type x, Mesh::size_type y, Mesh::size_type z);
    MeshHandle (*structquadsurf_maker)(Mesh::size_type x, Mesh::size_type y);
    MeshHandle (*structcurve_maker)(Mesh::size_type x);

    
};


MeshHandle CreateMesh(const std::string& type);
MeshHandle CreateMesh(const std::string& type, Mesh::size_type x, Mesh::size_type y, Mesh::size_type z, const Core::Geometry::Point& min, const Core::Geometry::Point& max);
MeshHandle CreateMesh(const std::string& type, Mesh::size_type x, Mesh::size_type y, const Core::Geometry::Point& min, const Core::Geometry::Point& max);
MeshHandle CreateMesh(const std::string& type, Mesh::size_type x, const Core::Geometry::Point& min, const Core::Geometry::Point& max);
MeshHandle CreateMesh(const std::string& type, Mesh::size_type x, Mesh::size_type y, Mesh::size_type z);
MeshHandle CreateMesh(const std::string& type, Mesh::size_type x, Mesh::size_type y);
MeshHandle CreateMesh(const std::string& type, Mesh::size_type x);


//! General case locate, search each elem.
template <class INDEX, class MESH>
bool elem_locate(INDEX &elem,
     MESH &msh, const Core::Geometry::Point &p) 
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
