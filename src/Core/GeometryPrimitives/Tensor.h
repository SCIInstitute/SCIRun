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


///
///@file  Tensor.h
///@brief Symmetric, positive definite tensors (diffusion, conductivity)
///
///@author
///       David Weinstein
///       Department of Computer Science
///       University of Utah
///@date  March 2001
///

#ifndef Geometry_Tensor_h
#define Geometry_Tensor_h 1

#include <Core/Utils/Legacy/TypeDescription.h>
#include <Core/Utils/Legacy/Assert.h>
#include <Core/Containers/Array1.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/share.h>
#include <Eigen/Dense>

#include <iosfwd>
#include <vector>


namespace SCIRun {

  class Piostream;
  class TypeDescription;

  namespace Core {

    namespace Geometry {

class SCISHARE Tensor {

public:
  Tensor();
  Tensor(const Tensor&);
  explicit Tensor(int);
  explicit Tensor(double);
  Tensor(double, double, double, double, double, double);
  explicit Tensor(const Array1<double> &); // 6 values
  explicit Tensor(const std::vector<double> &); // 6 values
  explicit Tensor(const double *); // 6 values
  explicit Tensor(const double **);
  Tensor(const Vector&, const Vector&, const Vector&);
  Tensor& operator=(const Tensor&);
  Tensor& operator=(const double&);

  bool operator==(const Tensor&) const;
  bool operator!=(const Tensor&) const;

  Tensor operator+(const Tensor&) const;
  Tensor& operator+=(const Tensor&);
  Tensor operator-(const Tensor&) const;
  Tensor& operator-=(const Tensor&);
  Tensor operator*(const double) const;
  Vector operator*(const Vector&) const;
  Tensor operator/(const double) const;

  static std::string type_name(int i = -1);

  void build_mat_from_eigens();
  void build_eigens_from_mat();
  void reorderTensorValues();
  void get_eigenvectors(Vector &e1, Vector &e2, Vector &e3);
  const Vector get_eigenvector1();
  const Vector get_eigenvector2();
  const Vector get_eigenvector3();
  void get_eigenvalues(double &l1, double &l2, double &l3);
  Eigen::MatrixXd mandel();

  double norm() const;
  Vector normalized_eigvals() const;
  void normal();
  double magnitude();

  void set_eigens(const Vector &e1, const Vector &e2, const Vector &e3);

  // This directly sets the eigenvectors and values in the tensor.  It
  // is meant to be used in conjunction with custom eigenvector/value
  // computation, such as that found in the TEEM package.
  void set_outside_eigens(const Vector &e1, const Vector &e2,
			  const Vector &e3,
			  double v1, double v2, double v3);

  /// support dynamic compilation
  static const std::string& get_h_file_path();

  friend SCISHARE void Pio(Piostream&, Tensor&);

  double xx() const { return mat_[0][0]; }
  double xy() const { return mat_[1][0]; }
  double xz() const { return mat_[2][0]; }
  double yy() const { return mat_[1][1]; }
  double yz() const { return mat_[2][1]; }
  double zz() const { return mat_[2][2]; }

  SCISHARE friend std::ostream& operator<<(std::ostream& os, const Tensor& t);
  SCISHARE friend std::istream& operator>>(std::istream& os, Tensor& t);

  double val(size_t i, size_t j) const { return mat_[i][j]; }
  double& val(size_t i, size_t j) { return mat_[i][j]; }

  double linearCertainty() const;
  double planarCertainty() const;
  double sphericalCertainty() const;

private:
  const static int DIM_ = 3;
  const static int SIZE_ = DIM_ * DIM_;
  double eigenValueSum() const;
  double mat_[DIM_][DIM_];
  Vector e1_, e2_, e3_;  // these are already scaled by the eigenvalues
  double l1_, l2_, l3_;
  int have_eigens_; //do NOT change to bool, it will break Pio system
};

SCISHARE void Pio(Piostream&, Tensor&);

inline bool operator<(const Tensor& t1, const Tensor& t2)
{
  return(t1.norm()<t2.norm());
}

inline bool operator<=(const Tensor& t1, const Tensor& t2)
{
  return(t1.norm()<=t2.norm());
}

inline bool operator>(const Tensor& t1, const Tensor& t2)
{
  return(t1.norm()>t2.norm());
}

inline bool operator>=(const Tensor& t1, const Tensor& t2)
{
  return(t1.norm()>=t2.norm());
}

inline
Tensor operator*(double d, const Tensor &t) {
  return t*d;
}

template <typename Indexable>
Tensor symmetricTensorFromSixElementArray(const Indexable& array)
{
  return Tensor(
    array[0],
    array[1],
    array[2],
    array[3],
    array[4],
    array[5]
    );
}

template <typename Indexable>
Tensor symmetricTensorFromMandel(const Indexable& array)
{
  const static double sqrt2 = std::sqrt(2);
  // std::vector<Vector> eigvecs(3);
  // eigvecs[0] = Vector(        array(0), sqrt2 * array(3), sqrt2 * array(4));
  // eigvecs[1] = Vector(sqrt2 * array(3),         array(1), sqrt2 * array(5));
  // eigvecs[2] = Vector(sqrt2 * array(4), sqrt2 * array(5),         array(2));
  return Tensor(array(0),
                array(3)/sqrt2,
                array(4)/sqrt2,
                array(1),
                array(5)/sqrt2,
                array(2));
  // return Tensor(eigvecs[0], eigvecs[1], eigvecs[2]);
}

template <typename Indexable>
Tensor symmetricTensorFromNineElementArray(const Indexable& array)
{
  static int sixElementTensorMatrixIndices[] = { 0, 1, 2, 4, 5, 8 };
  return Tensor(
    array[sixElementTensorMatrixIndices[0]],
    array[sixElementTensorMatrixIndices[1]],
    array[sixElementTensorMatrixIndices[2]],
    array[sixElementTensorMatrixIndices[3]],
    array[sixElementTensorMatrixIndices[4]],
    array[sixElementTensorMatrixIndices[5]]
    );
}

SCISHARE const TypeDescription* get_type_description(Tensor*);
    }}



} // End namespace SCIRun

#endif // Geometry_Tensor_h
