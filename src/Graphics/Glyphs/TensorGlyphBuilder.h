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


#ifndef Graphics_Glyphs_TENSOR_GLYPH_BUILDER_H
#define Graphics_Glyphs_TENSOR_GLYPH_BUILDER_H

#include <Core/Datatypes/Dyadic3DTensor.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Color.h>
#include <Core/Math/TrigTable.h>
#include <Graphics/Glyphs/GlyphConstructor.h>
#include <Graphics/Glyphs/share.h>

namespace SCIRun {
namespace Graphics {
  struct SCISHARE EllipsoidPointParams
  {
    double sinPhi;
    double cosPhi;
    double sinTheta;
    double cosTheta;
  };

  struct SCISHARE SuperquadricPointParams
  {
    double sinPhi;
    double cosPhi;
    double sinTheta;
    double cosTheta;
    double A;
    double B;
  };

class SCISHARE TensorGlyphBuilder
{
public:
  TensorGlyphBuilder(const Core::Datatypes::Dyadic3DTensor& t, const Core::Geometry::Point& center);
  void scaleTensor(double scale);
  void reorderTensorValues(std::vector<Core::Geometry::Vector>& eigvecs, std::vector<double>& eigvals);
  void makeTensorPositive();
  void normalizeTensor();
  void setShowNormals(bool showNormals);
  void setShowNormalsScale(double scale);
  void setColor(const Core::Datatypes::ColorRGB& color);
  void setResolution(double resolution);
  void generateSuperquadricTensor(GlyphConstructor& constructor, double emphasis);
  void generateSuperquadricSurface(GlyphConstructor& constructor, double A, double B);
  void generateEllipsoid(GlyphConstructor& constructor, bool half);
  void generateBox(GlyphConstructor& constructor);

protected:
  void generateSuperquadricSurfacePrivate(GlyphConstructor& constructor, double A, double B);
  Core::Geometry::Vector evaluateSuperquadricNormalLinear(const SuperquadricPointParams& params);
  Core::Geometry::Vector evaluateSuperquadricNormalPlanar(const SuperquadricPointParams& params);
  Core::Geometry::Point evaluateSuperquadricPointLinear(const SuperquadricPointParams& params);
  Core::Geometry::Point evaluateSuperquadricPointPlanar(const SuperquadricPointParams& params);
  Core::Geometry::Point evaluateEllipsoidPoint(const EllipsoidPointParams& params);
  Core::Geometry::Vector evaluateSuperquadricNormal(bool linear, const SuperquadricPointParams& params);
  Core::Geometry::Point evaluateSuperquadricPoint(bool linear, const SuperquadricPointParams& params);
  void generateBoxSide(GlyphConstructor& constructor, const Core::Geometry::Vector& p1, const Core::Geometry::Vector& p2,
                       const Core::Geometry::Vector& p3, const Core::Geometry::Vector& p4,
                       const Core::Geometry::Vector& normal);
  std::vector<Core::Geometry::Vector> generateBoxPoints();
  std::vector<Core::Geometry::Vector> getEigenVectors();
  std::vector<double> getEigenValues();
  void computeTransforms();
  void postScaleTransforms();
  void computeSinCosTable(bool half);

  bool showNormals_ = false;
  double normalDebugScale_ = 0.1;
  const static int DIMENSIONS_ = 3;
  const static int BOX_FACE_POINTS_ = 4;
  const static int DEGREES_OF_FREEDOM_ = 6;
  Graphics::Datatypes::SpireIBO::PRIMITIVE PRIM_ = Datatypes::SpireIBO::PRIMITIVE::TRIANGLES;
  Core::Datatypes::Dyadic3DTensor t_;
  Core::Geometry::Point center_;
  Core::Geometry::Transform trans_, rotate_;
  Core::Datatypes::ColorRGB color_ = {1.0, 1.0, 1.0};
  int resolution_ = 10;
  bool flatTensor_ = false;
  Core::Geometry::Vector zeroNorm_ = {0, 0, 0};
  SinCosTable tab1_, tab2_;
  int nv_ = 0;
  int nu_ = 0;
};

  using MandelVector = Eigen::Matrix<double, 6, 1>;

  struct DifftValues
  {
    const static size_t size = 12;
    std::array<Eigen::Matrix3d, size> undoScaleAndRotate;
    std::array<bool, size> linear;
    std::array<double, size> A;
    std::array<double, size> B;
    std::array<Eigen::Vector3d, size> normEigvals;
  };

  class SCISHARE UncertaintyTensorOffsetSurfaceBuilder : public TensorGlyphBuilder
  {
   public:
    UncertaintyTensorOffsetSurfaceBuilder(const Core::Datatypes::Dyadic3DTensor& t,
        const Core::Geometry::Point& center, double emphasis, const Eigen::Matrix<double, 6, 6>& covarianceMatrix);
    void generateOffsetSurface(GlyphConstructor& constructor);
    void precalculateDifftValues(DifftValues& vals, const MandelVector& t);
    void scaleCovariance(double scale);

   private:
    double evaluateSuperquadricImpl(bool linear, const Eigen::Vector3d& p, double A, double B);

    double evaluateSuperquadricImplLinear(const Eigen::Vector3d& p, double A, double B);
    double evaluateSuperquadricImplPlanar(const Eigen::Vector3d& p, double A, double B);
    MandelVector getQn(const DifftValues& vals, const Eigen::Vector3d& p);
    double emphasis_ = 0.0;
    double h_;// = 0.000001;
    double hHalf_;// = 0.5 * h_;
    Eigen::Matrix<double, 6, 6> covarianceMatrix_;

    //temp
    double diffT(const Eigen::Matrix<double, 6, 1>& s1,
                 const Eigen::Matrix<double, 6, 1>& s2,
                 const Eigen::Vector3d& p, double emphasis);
  };
}}

#endif
