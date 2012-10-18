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

#ifndef CORE_DATATYPES_FIELD_H
#define CORE_DATATYPES_FIELD_H 

#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Mesh/FieldFwd.h>
#include <boost/function.hpp>
//#include <Core/Datatypes/Mesh.h>
//#include <Core/Datatypes/VMesh.h>
//#include <Core/Datatypes/VField.h>
//#include <Core/Containers/LockingHandle.h>
//
//#include <Core/Util/Debug.h>
//#include <Core/Util/ProgressReporter.h>

#include <Core/Datatypes/Mesh/Share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {
 
class SCISHARE Field : public Datatype
{
  public:
    Field();
    Field(const Field& copy);// : PropertyManager(copy)       { DEBUG_CONSTRUCTOR("Field");  }
    virtual ~Field();
    
    virtual Field* clone() const = 0;

    //! Get pointers to associated structures
    //! mesh -> handle to mesh
    //! vmesh -> handle to virtual mesh interface
    //! field -> handle to this object
    //! vfield -> handle to virtual field interface
    virtual MeshHandle mesh() const = 0;
    FieldHandle field();// { return (this); }

    virtual VirtualMeshHandle vmesh()   const = 0;
    virtual VirtualFieldHandle vfield() const = 0;
    
    //! Detach the mesh from the field, if needed make a new copy of it.
    virtual void mesh_detach() = 0;

    //! The order of the field: we could get this one from the type_description
    virtual int basis_order() const = 0;

    //! Type Description to retrieve information on the actual type of the field
    enum td_info_e 
    {
      FULL_TD_E,
      FIELD_NAME_ONLY_E,
      MESH_TD_E,
      BASIS_TD_E,
      FDATA_TD_E
    };

    virtual const TypeDescription* get_type_description(td_info_e td = FULL_TD_E) const = 0; 
    
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    //! Persistent I/O.
    static  PersistentTypeID type_id;
    virtual void io(Piostream &stream);
    virtual std::string type_name() const { return type_id.type; }    
#endif
};

typedef boost::function<FieldHandle()> FieldMaker;
typedef boost::function<FieldHandle(MeshHandle)> FieldMakerFromMesh;

class SCISHARE FieldTypeID 
{
  public:
    // Constructor
    FieldTypeID(const std::string& type, 
                FieldMaker fieldMaker,
                FieldMakerFromMesh field_maker_mesh);
    
    std::string type;
    FieldMaker field_maker;
    FieldMakerFromMesh field_maker_mesh;
};


//TODO: FieldFactory class
SCISHARE FieldHandle CreateField(const std::string& type);
SCISHARE FieldHandle CreateField(const std::string& type, MeshHandle mesh);

}}}

#endif

