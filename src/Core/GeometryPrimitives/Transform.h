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

#ifndef CORE_GEOMETRY_TRANSFORM_H
#define CORE_GEOMETRY_TRANSFORM_H 1

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
//#include <Core/Geometry/Plane.h>
//#include <Core/Geometry/Tensor.h>
//#include <Core/Persistent/Persistent.h>

#include <Core/GeometryPrimitives/Share.h>

namespace SCIRun {
namespace Core {
namespace Geometry {

class SCISHARE Transform  //: public Persistent
{
  private:
    double mat[4][4];
    mutable double imat[4][4];
    mutable bool inverse_valid;

    void install_mat(double[4][4]);
    void build_permute(double m[4][4], int, int, int, int pre);
    void build_rotate(double m[4][4], double, const Geometry::Vector&);
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    void build_shear(double mat[4][4], const Geometry::Vector&, const Plane&);
#endif
    void build_scale(double m[4][4], const Geometry::Vector&);
    void build_translate(double m[4][4], const Geometry::Vector&);
    void pre_mulmat(const double[4][4]);
    void post_mulmat(const double[4][4]);
    void invmat(double[4][4]);
    void switch_rows(double m[4][4], int row1, int row2) const;
    void sub_rows(double m[4][4], int row1, int row2, double mul) const;
    void load_identity(double[4][4]);
    void load_zero(double[4][4]);

    double get_imat_val(int i, int j) const { return imat[i][j]; }
    void set_imat_val(int i, int j, double val) { imat[i][j] = val; }

  public:
    Transform();
    Transform(const Transform&);
    Transform& operator=(const Transform&);
    ~Transform();
    Transform(const Geometry::Point&, const Geometry::Vector&, const Geometry::Vector&, const Geometry::Vector&);

    double get_mat_val(int i, int j) const { return mat[i][j]; }
    void set_mat_val(int i, int j, double val) { mat[i][j] = val; }

    void load_basis(const Geometry::Point&,const Geometry::Vector&, const Geometry::Vector&, const Geometry::Vector&);
    void load_frame(const Geometry::Vector&, const Geometry::Vector&, const Geometry::Vector&);

    void change_basis(Transform&);
    void post_trans(const Transform&);
    void pre_trans(const Transform&);
      
    void print();
    void printi();

    void pre_permute(int xmap, int ymap, int zmap);
    void post_permute(int xmap, int ymap, int zmap);
    void pre_scale(const Geometry::Vector&);
    void post_scale(const Geometry::Vector&);
    
    void load_identity();
    // Returns true if the rotation happened, false otherwise.
    bool rotate(const Geometry::Vector& from, const Geometry::Vector& to);
    void pre_translate(const Geometry::Vector&);
    void post_translate(const Geometry::Vector&);
    
    void compute_imat() const;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    void pre_shear(const Geometry::Vector&, const Plane&);
    void post_shear(const Geometry::Vector&, const Plane&);

    void pre_rotate(double, const Geometry::Vector& axis);
    void post_rotate(double, const Geometry::Vector& axis);
    
    

    Geometry::Point unproject(const Geometry::Point& p) const;
    void unproject(const Geometry::Point& p, Geometry::Point& res) const;
    void unproject_inplace(Geometry::Point& p) const;
    Geometry::Vector unproject(const Geometry::Vector& p) const;
    void unproject(const Geometry::Vector& v, Geometry::Vector& res) const;
    void unproject_inplace(Geometry::Vector& v) const;
    Geometry::Point project(const Geometry::Point& p) const;
    void project(const Geometry::Point& p, Geometry::Point& res) const;
    void project_inplace(Geometry::Point& p) const;
    Geometry::Vector project(const Geometry::Vector& p) const;
    void project(const Geometry::Vector& p, Geometry::Vector& res) const;
    void project_inplace(Geometry::Vector& p) const;
    Geometry::Vector project_normal(const Geometry::Vector&) const;
    void project_normal(const Geometry::Vector&, Geometry::Vector& res) const;
    void project_normal_inplace(Geometry::Vector&) const;
    void get(double*) const;
    void get_trans(double*) const;
    void set(double*);
    void set_trans(double*);
    
    void perspective(const Geometry::Point& eyep, const Geometry::Point& lookat,
         const Geometry::Vector& up, double fov,
         double znear, double zfar,
         int xres, int yres);
    
    void invert();
    bool inv_valid() { return inverse_valid; }
    void set_inv_valid(bool iv) { inverse_valid = iv; }
#endif

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    //! Persistent I/O.
    static PersistentTypeID type_id;
    virtual void io(Piostream &stream);
    SCISHARE friend void Pio(Piostream&, Transform*&);
#endif
};

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
SCISHARE Geometry::Point operator*(Transform &t, const Geometry::Point &d);
SCISHARE Geometry::Vector operator*(Transform &t, const Geometry::Vector &d);

SCISHARE SCIRun::Tensor operator*(const SCIRun::Transform &t, const SCIRun::Tensor &d);
SCISHARE SCIRun::Tensor operator*(const SCIRun::Tensor &d, const SCIRun::Transform &t);

SCISHARE void Pio(Piostream&, Transform*&);
SCISHARE const TypeDescription* get_type_description(Transform*);
#endif

}}}

#endif
