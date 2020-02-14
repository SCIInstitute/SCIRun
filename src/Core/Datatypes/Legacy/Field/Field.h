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


#ifndef CORE_DATATYPES_LEGACY_FIELD_H
#define CORE_DATATYPES_LEGACY_FIELD_H 1

#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Legacy/Field/FieldFwd.h>
#include <Core/Datatypes/PropertyManagerExtensions.h>
#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

class SCISHARE Field : public Core::Datatypes::Datatype, public Core::Datatypes::HasPropertyManager
{
  public:
    Field();
    Field(const Field& copy);
    virtual ~Field();

    /// Clone field will generate a pointer to a new copy
    virtual Field* clone() const = 0;

    virtual Field* deep_clone() const = 0;

    /// Get pointers to associated structures
    /// mesh -> handle to mesh
    /// vmesh -> handle to virtual mesh interface
    /// vfield -> handle to virtual field interface
    virtual MeshHandle mesh() const = 0;

    virtual VMesh* vmesh()   const = 0;
    virtual VField* vfield() const = 0;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    /// Detach the mesh from the field, if needed make a new copy of it.
    // NOTE: IF THIS FUNCTION IS CALLED IN LEGACY CODE, IT MUST BE CONVERTED TO A deep_clone CALL ON THE FIELD OBJECT
    // FYI detach() calls should be converted to clone() calls (shallow copy, only field data is copied, not the mesh).
    virtual void mesh_detach() = 0;
#endif
    /// The order of the field: we could get this one from the type_description
    virtual int basis_order() const = 0;

    /// Type Description to retrieve information on the actual type of the field
    enum  td_info_e {
      FULL_TD_E,
      FIELD_NAME_ONLY_E,
      MESH_TD_E,
      BASIS_TD_E,
      FDATA_TD_E
    };

    virtual const TypeDescription* get_type_description(td_info_e td = FULL_TD_E) const = 0;

    /// Persistent I/O.
    static  PersistentTypeID type_id;
    virtual void io(Piostream &stream);
    virtual std::string type_name() const;
};


class SCISHARE FieldTypeID {
  public:
    // Constructor
    FieldTypeID(const std::string& type,
                FieldHandle (*field_maker)(),
                FieldHandle (*field_maker_mesh)(MeshHandle));

    std::string type;
    FieldHandle (*field_maker)();
    FieldHandle (*field_maker_mesh)(MeshHandle);
};

SCISHARE FieldHandle CreateField(const std::string& type);
SCISHARE FieldHandle CreateField(const std::string& type,MeshHandle mesh);

SCISHARE FieldHandle CreateField(const std::string& meshtype, const std::string& basistype, const std::string& datatype);
SCISHARE FieldHandle CreateField(const std::string& meshtype, const std::string& meshbasistype, const std::string& databasistype, const std::string& datatype);

SCISHARE FieldHandle CreateField(mesh_info_type mesh,
  meshbasis_info_type meshbasis,
  databasis_info_type databasis,
  data_info_type data);

SCISHARE FieldHandle CreateField(mesh_info_type mesh,
  databasis_info_type databasis,
  data_info_type data);

SCISHARE FieldHandle CreateField(FieldInformation &info);
SCISHARE FieldHandle CreateField(FieldInformation &info,MeshHandle mesh);

class SCISHARE NullField : public Field
{
public:
  explicit NullField(const std::string& name = "null") : Field() {}
  virtual Field* clone() const { return 0; }
  virtual Field* deep_clone() const { return 0; }
  virtual MeshHandle mesh() const { return MeshHandle(); }
  virtual VMesh* vmesh()   const { return 0; }
  virtual VField* vfield() const { return 0; }
  virtual int basis_order() const { return 0; }
  virtual const TypeDescription* get_type_description(td_info_e td) const { return 0; }
  virtual std::string dynamic_type_name() const { return "NullField"; }
};

}

#endif
