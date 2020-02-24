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


/// @todo Documentation Core/Datatypes/Legacy/Field/FieldInformation.h

#ifndef CORE_DATATYPES_FIELDINFORMATION
#define CORE_DATATYPES_FIELDINFORMATION 1

#include <string>
#include <vector>
#include <Core/Utils/Exception.h>
#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

class SCISHARE FieldTypeInformation
{

  public:

    bool        is_isomorphic() const;
    bool        is_nonlinear() const;
    bool        is_linear() const;

    bool        is_nodata() const;
    bool        is_constantdata() const;
    bool        is_lineardata() const;
    bool        is_nonlineardata() const;
    bool        is_quadraticdata() const;
    bool        is_cubicdata() const;
    int         field_basis_order() const;

    bool        is_constantmesh() const;
    bool        is_linearmesh() const;
    bool        is_nonlinearmesh() const;
    bool        is_quadraticmesh() const;
    bool        is_cubicmesh() const;
    int         mesh_basis_order() const;

    bool        is_tensor() const;
    bool        is_vector() const;
    bool        is_scalar() const;
    bool        is_integer() const;
    bool        is_double() const;
    bool        is_float() const;
    bool        is_longlong() const;
    bool        is_long() const;
    bool        is_int() const;
    bool        is_short() const;
    bool        is_char() const;
    bool        is_unsigned_longlong() const;
    bool        is_unsigned_long() const;
    bool        is_unsigned_int() const;
    bool        is_unsigned_short() const;
    bool        is_unsigned_char() const;
    bool        is_dvt() const; // double vector tensor
    bool        is_svt() const; // scalar vector tensor

    bool        is_regularmesh() const;
    bool        is_irregularmesh() const;
    bool        is_structuredmesh() const;
    bool        is_unstructuredmesh() const;

    // These should go...
    inline bool is_pointcloud() const { return(is_pointcloudmesh()); }
    inline bool is_scanline() const { return(is_scanlinemesh()); }
    inline bool is_image() const { return(is_imagemesh()); }
    inline bool is_latvol() const { return(is_latvolmesh()); }
    inline bool is_curve() const { return(is_curvemesh()); }
    inline bool is_trisurf() const { return(is_trisurfmesh()); }
    inline bool is_quadsurf() const { return(is_quadsurfmesh()); }
    inline bool is_tetvol() const { return(is_tetvolmesh()); }
    inline bool is_prismvol() const { return(is_prismvolmesh()); }
    inline bool is_hexvol() const { return(is_hexvolmesh()); }
    inline bool is_structcurve() const { return(is_structcurvemesh()); }
    inline bool is_structquadsurf() const { return(is_structquadsurfmesh()); }
    inline bool is_structhexvol() const { return(is_structhexvolmesh()); }

    // Naming that is more systematic
    bool        is_pointcloudmesh() const;
    bool        is_scanlinemesh() const;
    bool        is_imagemesh() const;
    bool        is_latvolmesh() const;
    bool        is_curvemesh() const;
    bool        is_trisurfmesh() const;
    bool        is_quadsurfmesh() const;
    bool        is_tetvolmesh() const;
    bool        is_prismvolmesh() const;
    bool        is_hexvolmesh() const;
    bool        is_structcurvemesh() const;
    bool        is_structquadsurfmesh() const;
    bool        is_structhexvolmesh() const;


    bool        is_point() const;
    bool        is_line() const;
    bool        is_surface() const;
    bool        is_volume() const;

    bool        is_pnt_element() const;
    bool        is_crv_element() const;
    bool        is_tri_element() const;
    bool        is_quad_element() const;
    bool        is_tet_element() const;
    bool        is_prism_element() const;
    bool        is_hex_element() const;

    void insert_field_type_information(Field* field);
  protected:

    // type names
    std::string field_type;
    std::string mesh_type;
    std::string mesh_basis_type;
    std::string point_type;
    std::string basis_type;
    std::string data_type;
    std::string container_type;

};


class SCISHARE FieldInformation : public FieldTypeInformation {

  public:

    FieldInformation(const std::string& meshtype,const std::string& meshbasis,
                                  const std::string& databasis, const std::string& datatype);
    FieldInformation(const std::string& meshtype,const std::string& basis, const std::string& datatype);

    FieldInformation(const std::string& meshtype,int,int, const std::string& datatype);
    FieldInformation(const std::string& meshtype,int, const std::string& datatype);

    FieldInformation(mesh_info_type mesh, meshbasis_info_type databasis,
                     databasis_info_type, data_info_type data);
    FieldInformation(mesh_info_type mesh,databasis_info_type databasis,
                     data_info_type data);

    explicit FieldInformation(FieldHandle handle);
    explicit FieldInformation(Field* field);


    std::string get_field_type() const;
    void        set_field_type(const std::string&);

    std::string get_mesh_type() const;
    std::string get_mesh_type_id() const;
    void        set_mesh_type(const std::string&);
    void        set_mesh_type(mesh_info_type);

    std::string get_mesh_basis_type() const;
    void        set_mesh_basis_type(const std::string&);
    void        set_mesh_basis_type(int);

    std::string get_point_type() const;
    void        set_point_type(const std::string&);

    std::string get_basis_type() const;
    void        set_basis_type(const std::string&);
    void        set_basis_type(int);

    // alternative way of setting data_basis
    std::string get_data_basis_type() const { return (get_basis_type()); }
    void        set_data_basis_type(const std::string& s) { set_basis_type(s); }
    void        set_data_basis_type(int s) { set_basis_type(s); }

    std::string get_data_type() const;
    void        set_data_type(const std::string&);
    bool        set_data_type_by_string(const std::string& type);
    void        set_data_type(data_info_type);

    std::string get_container_type() const;
    void        set_container_type(const std::string&);

    std::string get_field_type_id() const;

    bool        make_nodata();
    bool        make_constantdata();
    bool        make_lineardata();
    bool        make_quadraticdata();
    bool        make_cubicdata();

    bool        make_constantmesh(); // for pointcloud only
    bool        make_linearmesh();
    bool        make_quadraticmesh();
    bool        make_cubicmesh();

    bool        make_char();
    bool        make_unsigned_char();
    bool        make_short();
    bool        make_unsigned_short();
    bool        make_int();
    bool        make_unsigned_int();
    bool        make_long();
    bool        make_unsigned_long();
    bool        make_long_long();
    bool        make_unsigned_long_long();
    bool        make_float();
    bool        make_scalar();
    bool        make_double();
    bool        make_vector();
    bool        make_tensor();

    bool        make_pointcloudmesh();
    bool        make_scanlinemesh();
    bool        make_imagemesh();
    bool        make_latvolmesh();
    bool        make_structcurvemesh();
    bool        make_structquadsurfmesh();
    bool        make_structhexvolmesh();
    bool        make_curvemesh();
    bool        make_trisurfmesh();
    bool        make_quadsurfmesh();
    bool        make_tetvolmesh();
    bool        make_prismvolmesh();
    bool        make_hexvolmesh();

    bool        make_unstructuredmesh();
    bool        make_irregularmesh();

    bool        operator==(const FieldInformation&) const;
    bool        operator!=(const FieldInformation&) const;

    // testing for the data type

    inline bool is_data_typeT(char* ) const               { return (is_char()); }
    inline bool is_data_typeT(unsigned char* ) const      { return (is_unsigned_char()); }
    inline bool is_data_typeT(short* ) const              { return (is_short()); }
    inline bool is_data_typeT(unsigned short* ) const     { return (is_unsigned_short()); }
    inline bool is_data_typeT(int* ) const                { return (is_int()); }
    inline bool is_data_typeT(unsigned int* ) const       { return (is_unsigned_int()); }
    inline bool is_data_typeT(long* ) const               { return (is_long()); }
    inline bool is_data_typeT(unsigned long* ) const      { return (is_unsigned_long()); }
    inline bool is_data_typeT(long long* ) const          { return (is_longlong()); }
    inline bool is_data_typeT(unsigned long long* ) const { return (is_unsigned_longlong()); }
    inline bool is_data_typeT(double* ) const             { return (is_double()); }
    inline bool is_data_typeT(float* ) const              { return (is_float()); }
    inline bool is_data_typeT(Core::Geometry::Vector* ) const             { return (is_vector()); }
    inline bool is_data_typeT(Core::Geometry::Tensor* ) const             { return (is_tensor()); }
    template<class T> bool is_data_typeT(T*) const        { return (false); }

    inline void set_data_typeT(char*) { make_char(); }
    inline void set_data_typeT(unsigned char*) { make_unsigned_char(); }
    inline void set_data_typeT(short*) { make_short(); }
    inline void set_data_typeT(unsigned short*) { make_unsigned_short(); }
    inline void set_data_typeT(int*) { make_int(); }
    inline void set_data_typeT(unsigned int*) { make_unsigned_int(); }
    inline void set_data_typeT(long*) { make_long(); }
    inline void set_data_typeT(unsigned long*) { make_unsigned_long(); }
    inline void set_data_typeT(long long*) { make_long_long(); }
    inline void set_data_typeT(unsigned long long*) { make_unsigned_long_long(); }
    inline void set_data_typeT(float*) { make_float(); }
    inline void set_data_typeT(double*) { make_double(); }
    inline void set_data_typeT(Core::Geometry::Vector*) { make_vector(); }
    inline void set_data_typeT(Core::Geometry::Tensor*) { make_tensor(); }
};

struct SCISHARE MeshException : virtual SCIRun::Core::ExceptionBase {};

struct SCISHARE UnknownMeshType : virtual MeshException {};

inline bool UseIntegerInterface(FieldInformation &info)
{ return(info.is_integer()); }

inline bool UseIntegerInterface(FieldInformation &info,FieldInformation &info2)
{ return(info.is_integer()&info2.is_integer()); }

inline bool UseIntegerInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3)
{ return(info.is_integer()&info2.is_integer()&info3.is_integer()); }

inline bool UseIntegerInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4)
{ return(info.is_integer()&info2.is_integer()&info3.is_integer()&info4.is_integer()); }

inline bool UseIntegerInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4, FieldInformation &info5)
{ return(info.is_integer()&info2.is_integer()&info3.is_integer()&info4.is_integer()&info5.is_integer()); }

inline bool UseScalarInterface(FieldInformation &info)
{ return(info.is_scalar()); }

inline bool UseScalarInterface(FieldInformation &info,FieldInformation &info2)
{ return(info.is_scalar()&info2.is_scalar()); }

inline bool UseScalarInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3)
{ return(info.is_scalar()&info2.is_scalar()&info3.is_scalar()); }

inline bool UseScalarInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4)
{ return(info.is_scalar()&info2.is_scalar()&info3.is_scalar()&info4.is_scalar()); }

inline bool UseScalarInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4, FieldInformation &info5)
{ return(info.is_scalar()&info2.is_scalar()&info3.is_scalar()&info4.is_scalar()&info5.is_scalar()); }


inline bool UseVectorInterface(FieldInformation &info)
{ return(info.is_vector()); }

inline bool UseVectorInterface(FieldInformation &info,FieldInformation &info2)
{ return(info.is_vector()&info2.is_vector()); }

inline bool UseVectorInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3)
{ return(info.is_vector()&info2.is_vector()&info3.is_vector()); }

inline bool UseVectorInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4)
{ return(info.is_vector()&info2.is_vector()&info3.is_vector()&info4.is_vector()); }

inline bool UseVectorInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4, FieldInformation &info5)
{ return(info.is_vector()&info2.is_vector()&info3.is_vector()&info4.is_vector()&info5.is_vector()); }


inline bool UseTensorInterface(FieldInformation &info)
{ return(info.is_tensor()); }

inline bool UseTensorInterface(FieldInformation &info,FieldInformation &info2)
{ return(info.is_tensor()&info2.is_tensor()); }

inline bool UseTensorInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3)
{ return(info.is_tensor()&info2.is_tensor()&info3.is_tensor()); }

inline bool UseTensorInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4)
{ return(info.is_tensor()&info2.is_tensor()&info3.is_tensor()&info4.is_tensor()); }

inline bool UseTensorInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4, FieldInformation &info5)
{ return(info.is_tensor()&info2.is_tensor()&info3.is_tensor()&info4.is_tensor()&info5.is_tensor()); }

} // end namespace

#endif
