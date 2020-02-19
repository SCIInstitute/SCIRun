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


#ifndef CORE_DATATYPES_GENERICFIELD_H
#define CORE_DATATYPES_GENERICFIELD_H 1

#include <Core/Utils/Legacy/Debug.h>
#include <Core/Basis/Locate.h>
#include <Core/Containers/StackVector.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VFData.h>
#include <Core/Datatypes/Legacy/Base/TypeName.h>
#include <Core/Datatypes/Legacy/Field/MeshTypes.h>

#include <Core/Persistent/PersistentSTL.h>
#include <Core/Containers/FData.h>
#include <Core/Datatypes/Legacy/Field/CastFData.h>
#include <Core/Containers/StackVector.h>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

template <class Mesh, class Basis, class FData>
class GenericField: public Field
{
public:
  /// Typedefs to support the Field concept.
  typedef GenericField<Mesh, Basis, FData>                 field_type;
  typedef typename FData::value_type                       value_type;
  typedef Mesh                                             mesh_type;
  typedef boost::shared_ptr<mesh_type>                         mesh_handle_type;
  typedef Basis                                            basis_type;
  typedef FData                                            fdata_type;
  typedef boost::shared_ptr<GenericField<Mesh, Basis, FData> > handle_type;
  typedef SCIRun::index_type                               index_type;
  typedef SCIRun::size_type                                size_type;

  /// only Pio should use this constructor
  GenericField();
  /// Use this constructor to actually have a field with a mesh
  GenericField(mesh_handle_type mesh);
  GenericField(const GenericField &copy);

  virtual ~GenericField();

  /// Clone the field data, but not the mesh.
  /// Use mesh_detach() first to clone the complete field
  virtual GenericField<Mesh, Basis, FData> *clone() const;

  /// Clone everything, field data and mesh.
  virtual GenericField<Mesh, Basis, FData> *deep_clone() const;

  /// Obtain a Handle to the Mesh
  virtual MeshHandle mesh() const;
  virtual VMesh*  vmesh() const;
  virtual VField* vfield() const;

  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  /// Clone the mesh
  virtual void mesh_detach();
  #endif

  /// Get the order of the field data
  /// -1 = no data
  /// 0 = constant data per element
  /// 1 = linear data per element
  /// >1 = non linear data per element
  virtual int basis_order() const { return basis_.polynomial_order(); }

  /// Get the classes on which this function relies:
  /// Get the basis describing interpolation within an element
  Basis& get_basis()  { return basis_; }

  /// Get the mesh describing how the elements fit together
  // const mesh_handle_type &get_typed_mesh() const;

  /// Persistent I/O.
  virtual void io(Piostream &stream);

  /// Tag the constructor of this class and put it in the Pio DataBase
  static  PersistentTypeID type_id;

  /// Tag the constructor of this class and put it in the Field DataBase
  static  FieldTypeID field_id;

  /// Function to retrieve the name of this field class
  static  const std::string type_name(int n = -1);
  virtual std::string dynamic_type_name() const { return type_id.type; }

  /// A different way of tagging a class. Currently two systems are used next
  /// to each other: type_name and get_type_description. Neither is perfect
  virtual
  const TypeDescription* get_type_description(td_info_e td = FULL_TD_E) const;

  /// Static functions to instantiate the field from Pio or using CreateField()
  static Persistent *maker();
  static FieldHandle field_maker();
  static FieldHandle field_maker_mesh(MeshHandle mesh);

protected:

  /// A (generic) mesh.
  mesh_handle_type             mesh_;
  /// Data container.
  fdata_type                   fdata_;
  Basis                        basis_;

  VField*                      vfield_;

  int basis_order_;
  int mesh_dimensionality_;
};


template<class FIELD>
class VGenericField : public VField {
  public:
    VGenericField(FIELD* field, VFData* vfdata)
    {
      DEBUG_CONSTRUCTOR("VGenericField")

      field_ = field;
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      pm_ = field;
#endif
      vfdata_ = vfdata;
      mesh_ = field_->mesh().get();
      vmesh_ = mesh_->vmesh();
      basis_order_ = field->basis_order();
      number_of_nodes_ = field->get_basis().number_of_mesh_vertices();
      number_of_enodes_ = field->get_basis().number_of_vertices() - number_of_nodes_;
      element_dim_ = field->get_basis().domain_dimension();
      element_dofs_ = field->get_basis().dofs();
      data_type_ = find_type_name(static_cast<typename FIELD::value_type*>(0));
      for (size_t j=0; j<data_type_.size(); j++)
        if(data_type_[j] == '_') data_type_[j] = ' ';

      /// Create a fast way of checking scalar/pair/vector/tensor
      is_scalar_ = false;
      is_vector_ = false;
      is_tensor_ = false;
      is_pair_ = false;

      if (data_type_.substr(0,6) == "Vector") is_vector_ = true;
      else if (data_type_.substr(0,6) == "Tensor") is_tensor_ = true;
      else if (data_type_.substr(0,4) == "Pair") is_pair_ = true;
      else if (field->basis_order() > -1) is_scalar_ = true;
    }

    virtual ~VGenericField()
    {
      DEBUG_DESTRUCTOR("VGenericField")
      if (vfdata_) delete vfdata_;
    }

};

// PIO
const int GENERICFIELD_VERSION = 3;


template <class Mesh, class Basis, class FData>
Persistent *
GenericField<Mesh, Basis, FData>::maker()
{
  return new GenericField<Mesh, Basis, FData>;
}

template <class Mesh, class Basis, class FData>
FieldHandle
GenericField<Mesh, Basis, FData>::field_maker()
{
  return boost::make_shared<GenericField<Mesh, Basis, FData>>();
}


template <class Mesh, class Basis, class FData>
FieldHandle
GenericField<Mesh, Basis, FData>::field_maker_mesh(MeshHandle mesh)
{
  mesh_handle_type mesh_handle = boost::dynamic_pointer_cast<mesh_type>(mesh);
  if (mesh_handle)
    return boost::make_shared<GenericField<Mesh, Basis, FData>>(mesh_handle);
  else
    return FieldHandle();
}


template <class Mesh, class Basis, class FData>
PersistentTypeID
GenericField<Mesh, Basis, FData>::type_id(type_name(-1), "Field", maker);

template <class Mesh, class Basis, class FData>
FieldTypeID
GenericField<Mesh, Basis, FData>::field_id(type_name(-1),field_maker,field_maker_mesh);

template <class Mesh, class Basis, class FData>
void GenericField<Mesh, Basis, FData>::io(Piostream& stream)
{
  int version = stream.begin_class(type_name(), GENERICFIELD_VERSION);

  if (stream.backwards_compat_id())
  {
    version = stream.begin_class(type_name(), GENERICFIELD_VERSION);
  }
  Field::io(stream);

  if (stream.error()) return;

  if (version < 2)
    mesh_->io(stream);
  else
    Pio(stream, mesh_);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  mesh_->freeze();
#endif

  if (version >= 3)
  {
    basis_.io(stream);
  }

  Pio(stream, fdata_);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  freeze();
#endif

  if (stream.backwards_compat_id())
  {
    stream.end_class();
  }
  stream.end_class();

  // A new mesh is associated with it
  if (stream.reading())
  {
    vfield_->update_mesh_pointer(mesh_.get());
  }
}

template <class Mesh, class Basis, class FData>
GenericField<Mesh, Basis, FData>::GenericField() :
  Field(),
  mesh_(mesh_handle_type(new mesh_type())),
  fdata_(0),
  vfield_(0),
  basis_order_(0),
  mesh_dimensionality_(-1)
{
  DEBUG_CONSTRUCTOR("GenericField")

  basis_order_ = basis_order();
  if (mesh_) mesh_dimensionality_ = mesh_->dimensionality();

  VFData* vfdata = CreateVFData(fdata_,get_basis().get_nodes(),get_basis().get_derivs());
  vfield_ = new VGenericField<GenericField<Mesh,Basis,FData> >(this, vfdata);
  vfield_->resize_values();

}

template <class Mesh, class Basis, class FData>
GenericField<Mesh, Basis, FData>::GenericField(const GenericField& copy) :
  Field(copy),
  mesh_(copy.mesh_),
  fdata_(copy.fdata_),
  basis_(copy.basis_),
  vfield_(0),
  basis_order_(copy.basis_order_),
  mesh_dimensionality_(copy.mesh_dimensionality_)
{
  DEBUG_CONSTRUCTOR("GenericField")

  VFData* vfdata = CreateVFData(fdata_,get_basis().get_nodes(),get_basis().get_derivs());
  if (vfdata)
  {
    vfield_ = new VGenericField<GenericField<Mesh,Basis,FData> >(this, vfdata);
  }
}


template <class Mesh, class Basis, class FData>
GenericField<Mesh, Basis, FData>::GenericField(mesh_handle_type mesh) :
  Field(),
  mesh_(mesh),
  fdata_(0),
  vfield_(0),
  basis_order_(0),
  mesh_dimensionality_(-1)
{
  DEBUG_CONSTRUCTOR("GenericField")

  basis_order_ = basis_order();
  if (mesh_) mesh_dimensionality_ = mesh_->dimensionality();

  VFData* vfdata = CreateVFData(fdata_,get_basis().get_nodes(),get_basis().get_derivs());
  vfield_ = new VGenericField<GenericField<Mesh,Basis,FData> >(this, vfdata);
  vfield_->resize_values();
}


template <class Mesh, class Basis, class FData>
GenericField<Mesh, Basis, FData>::~GenericField()
{
  DEBUG_DESTRUCTOR("GenericField")
  if (vfield_) delete vfield_;
}

template <class Mesh, class Basis, class FData>
GenericField<Mesh, Basis, FData> *
GenericField<Mesh, Basis, FData>::clone() const
{
  return new GenericField<Mesh, Basis, FData>(*this);
}

template <class Mesh, class Basis, class FData>
GenericField<Mesh, Basis, FData> *
  GenericField<Mesh, Basis, FData>::deep_clone() const
{
  auto copy = new GenericField<Mesh, Basis, FData>(*this);
  copy->mesh_.reset(mesh_->clone());
  copy->vfield_->update_mesh_pointer(copy->mesh_.get());
  return copy;
}

template <class Mesh, class Basis, class FData>
MeshHandle
GenericField<Mesh, Basis, FData>::mesh() const
{
  return MeshHandle(mesh_);
}

template <class Mesh, class Basis, class FData>
VMesh*
GenericField<Mesh, Basis, FData>::vmesh() const
{
  if (mesh_) return mesh_->vmesh();
  return (0);
}

template <class Mesh, class Basis, class FData>
VField*
GenericField<Mesh, Basis, FData>::vfield() const
{
  return (vfield_);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
template <class Mesh, class Basis, class FData>
void
GenericField<Mesh, Basis, FData>::mesh_detach()
{
  thaw();
  mesh_.detach();
  mesh_->thaw();
  vfield_->update_mesh_pointer(mesh_.get_rep());
}
#endif

template <class Mesh, class Basis, class FData>
const std::string GenericField<Mesh, Basis, FData>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 3);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1), type_name(2), type_name(3));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("GenericField");
    return nm;
  }
  else if (n == 1)
  {
    return find_type_name(static_cast<Mesh *>(0));
  }
  else if (n == 2)
  {
    return find_type_name(static_cast<Basis *>(0));
  }
  else
  {
    return find_type_name(static_cast<FData *>(0));
  }
}

template <class Mesh, class Basis, class FData>
const TypeDescription *
GenericField<Mesh, Basis, FData>::get_type_description(td_info_e td) const
{
  static std::string name(type_name(0));
  static std::string namesp("SCIRun");
  static std::string path(__FILE__);
  const TypeDescription *sub1 = SCIRun::get_type_description(static_cast<Mesh*>(0));
  const TypeDescription *sub2 = SCIRun::get_type_description(static_cast<Basis*>(0));
  const TypeDescription *sub3 = SCIRun::get_type_description(static_cast<FData*>(0));

  switch (td) {
  default:
  case FULL_TD_E:
    {
      static TypeDescription* tdn1 = 0;
      if (tdn1 == 0) {
	TypeDescription::td_vec *subs = new TypeDescription::td_vec(3);
	(*subs)[0] = sub1;
	(*subs)[1] = sub2;
	(*subs)[2] = sub3;
	tdn1 = new TypeDescription(name, subs, path, namesp,
				      TypeDescription::FIELD_E);
      }
      return tdn1;
    }
  case FIELD_NAME_ONLY_E:
    {
      static TypeDescription* tdn0 = 0;
      if (tdn0 == 0) {
	tdn0 = new TypeDescription(name, 0, path, namesp,
				      TypeDescription::FIELD_E);
      }
      return tdn0;
    }
  case MESH_TD_E:
    {
      return sub1;
    }
  case BASIS_TD_E:
    {
      return sub2;
    }
  case FDATA_TD_E:
    {
      return sub3;
    }
  };
}

/// These ended up here, due to the problem with the include order in get_typedescription.
/// Once we have dismanteled that system this can go back to VFData
template<class T, class MESH>
inline VFData* CreateVFData(FData2d<T,MESH>& fdata, std::vector<T>& lfdata, std::vector<std::vector<T> >& hfdata)
{
  return (CreateVFData(static_cast<Array2<T>& >(fdata),lfdata,hfdata));
}

template<class T, class MESH>
inline VFData* CreateVFData(FData3d<T,MESH>& fdata, std::vector<T>& lfdata, std::vector<std::vector<T> >& hfdata)
{
  return (CreateVFData(static_cast<Array3<T>& >(fdata),lfdata,hfdata));
}

} // end namespace SCIRun






#endif // Datatypes_GenericField_h
