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


///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

/// @todo Documentation Core/GeometryPrimitives/Transform.h

#ifndef CORE_GEOMETRY_TRANSFORM_H
#define CORE_GEOMETRY_TRANSFORM_H 1

#include <vector>
#include <Core/Persistent/Persistent.h>

#include <Core/GeometryPrimitives/share.h>

namespace SCIRun {
  class TypeDescription;

  namespace Core {
    namespace Geometry {

      class Point;
      class Vector;
      class Plane;
      class Tensor;

      class SCISHARE Transform : public Persistent
      {
        double mat[4][4];
        mutable double imat[4][4];
        mutable bool inverse_valid;

        void install_mat(double[4][4]);
        void build_permute(double m[4][4], int, int, int, int pre);
        void build_rotate(double m[4][4], double, const Vector&);
        void build_shear(double mat[4][4], const Vector&, const Plane&);
        void build_scale(double m[4][4], double);
        void build_scale(double m[4][4], const Vector&);
        void build_translate(double m[4][4], const Vector&);
        void pre_mulmat(const double[4][4]);
        void post_mulmat(const double[4][4]);
        void switch_rows(double m[4][4], int row1, int row2) const;
        void sub_rows(double m[4][4], int row1, int row2, double mul) const;
        void load_identity(double[4][4]);
        void load_zero(double[4][4]);
        void gram_schmidt(bool normalize);
      public:
        double get_imat_val(int i, int j) const { return imat[i][j]; }
        void set_imat_val(int i, int j, double val) { imat[i][j] = val; }

        static const Transform& Identity();

        Transform();
        Transform(const Transform&);
        explicit Transform(const double* pmat);
        Transform& operator=(const Transform&);
        Transform(const Point&, const Vector&, const Vector&, const Vector&);

        double get_mat_val(int i, int j) const { return mat[i][j]; }
        void set_mat_val(int i, int j, double val) { mat[i][j] = val; }

        void load_basis(const Point&, const Vector&, const Vector&, const Vector&);
        void load_frame(const Vector&, const Vector&, const Vector&);

        void change_basis(Transform&);
        void change_basis(const Vector &, const Vector &, const Vector &);
        void post_trans(const Transform&);
        void pre_trans(const Transform&);

        void print();
        void printi();

        void pre_permute(int xmap, int ymap, int zmap);
        void post_permute(int xmap, int ymap, int zmap);
        void pre_scale(double d);
        void pre_scale(const Vector&);
        void post_scale(double d);
        void post_scale(const Vector&);

        void load_identity();
        // Returns true if the rotation happened, false otherwise.
        bool rotate(const Vector& from, const Vector& to);
        void pre_translate(const Vector&);
        void post_translate(const Vector&);

        void compute_imat() const;

        Vector project(const Vector& p) const;
        Point project(const Point& p) const;

        Point unproject(const Point& p) const;
        void unproject(const Point& p, Point& res) const;
        void unproject_inplace(Point& p) const;
        Vector unproject(const Vector& p) const;
        void unproject(const Vector& v, Vector& res) const;
        void unproject_inplace(Vector& v) const;

        void project(const Point& p, Point& res) const;
        void project_inplace(Point& p) const;

        void project(const Vector& p, Vector& res) const;
        void project_inplace(Vector& p) const;
        Vector project_normal(const Vector&) const;
        void project_normal(const Vector&, Vector& res) const;
        void project_normal_inplace(Vector&) const;

        void pre_shear(const Vector&, const Plane&);
        void post_shear(const Vector&, const Plane&);

        void pre_rotate(double, const Vector& axis);
        void post_rotate(double, const Vector& axis);

        void get(double*) const;
        std::string get_string() const;
        void get_trans(double*) const;
        void set(const double*);
        void set_trans(double*);

        void perspective(const Point& eyep, const Point& lookat,
          const Vector& up, double fov,
          double znear, double zfar,
          int xres, int yres);

        void invert();
        bool inv_valid() { return inverse_valid; }
        void set_inv_valid(bool iv) { inverse_valid = iv; }

        void orthogonalize();
        void orthonormalize();

        /// Persistent I/O.
        static PersistentTypeID type_id;
        virtual void io(Piostream &stream);
        std::vector<Vector> get_rotation() const;
        Point get_translation() const;
      };

      SCISHARE Transform transformFromString(const std::string& str);

      SCISHARE Point operator*(const Transform &t, const Point &d);
      SCISHARE Vector operator*(const Transform &t, const Vector &d);
      SCISHARE Transform operator*(const Transform &t1, const Transform &t2);

     SCISHARE Tensor operator*(const Transform &t, const Tensor &d);
      SCISHARE Tensor operator*(const Tensor &d, const Transform &t);

      SCISHARE std::istream& operator>>(std::istream& is, Transform& t);
      SCISHARE std::ostream& operator<<(std::ostream& is, const Transform& t);
      SCISHARE bool operator==(const Transform& lhs, const Transform& rhs);
      SCISHARE bool operator!=(const Transform& lhs, const Transform& rhs);

      SCISHARE void Pio_old(Piostream&, Transform&);
      SCISHARE const TypeDescription* get_type_description(Transform*);
    }
  }
}

#endif
