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

#ifndef CORE_DATATYPES_MESH_H
#define CORE_DATATYPES_MESH_H 

#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Mesh/MeshTraits.h>
#include <Core/Datatypes/Mesh/Share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  class SCISHARE Mesh5 : public Datatype, public MeshTraits
  {
  public: 
    Mesh5();
    Mesh5(const Mesh5& copy); //: PropertyManager(copy) { DEBUG_CONSTRUCTOR("Mesh5");  }
    virtual ~Mesh5();
    virtual Mesh5* clone() const = 0;
    //! Virtual interface functions:
    //! Get the virtual mesh interface, this returns the pointer to an internal
    //! object that has all the virtual functions. This object will be destroyed
    //! when the mesh is destroyed. The user does not need to destroy the VMesh.
    virtual VirtualMeshHandle vmesh() const;

    virtual MeshFacadeHandle getFacade() const = 0;

    virtual int basis_order();

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

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    virtual bool synchronize(mask_type) { return false; }
    virtual bool unsynchronize(mask_type) { return false; }
#endif

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    //! Persistent I/O.
    void    io(Piostream &stream);
    static  PersistentTypeID type_id;
    static  const std::string type_name(int n = -1);
    virtual const TypeDescription *get_type_description() const = 0;
#endif
  };

}}}

#endif

