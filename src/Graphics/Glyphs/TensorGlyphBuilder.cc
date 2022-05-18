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


#include <Graphics/Glyphs/TensorGlyphBuilder.h>
#include <Graphics/Glyphs/GlyphGeomUtility.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace Graphics;
using namespace Core::Geometry;
using namespace Core::Datatypes;
using GGU = GlyphGeomUtility;

TensorGlyphBuilder::TensorGlyphBuilder(const Dyadic3DTensor& t, const Point& center)
{
  t_ = t;
  center_ = Point(center);
}

void TensorGlyphBuilder::scaleTensor(double scale)
{
  t_ = t_ * scale;
}

void TensorGlyphBuilder::setShowNormals(bool showNormals)
{
  showNormals_ = showNormals;
}

void TensorGlyphBuilder::setShowNormalsScale(double scale)
{
  normalDebugScale_ = scale * t_.frobeniusNorm();
}

void TensorGlyphBuilder::normalizeTensor()
{
  t_.normalize();
}

void TensorGlyphBuilder::setColor(const ColorRGB& color)
{
  color_ = color;
}

void TensorGlyphBuilder::setResolution(double resolution)
{
  resolution_ = resolution;
}

void TensorGlyphBuilder::reorderTensorValues(std::vector<Vector>& eigvecs,
                                             std::vector<double>& eigvals)
{
  std::vector<std::pair<double, Vector>> sortList(3);
  for(int d = 0; d < DIMENSIONS_; ++d)
    sortList[d] = std::make_pair(eigvals[d], eigvecs[d]);

  std::sort(std::begin(sortList), std::end(sortList), std::greater<std::pair<double, Vector>>());

  for(int d = 0; d < DIMENSIONS_; ++d)
  {
    eigvals[d] = sortList[d].first;
    eigvecs[d] = sortList[d].second;
  }
}

void TensorGlyphBuilder::makeTensorPositive()
{
  static const double zeroThreshold = 0.000001;
  t_.makePositive();
  t_.setDescendingRHSOrder();
  auto eigvals = t_.getEigenvalues();
  auto eigvecs = t_.getEigenvectors();
  flatTensor_ = eigvals[2] < zeroThreshold;
  if (flatTensor_)
  {
    Eigen::Vector3d vec = eigvecs[0].cross(eigvecs[1]);
    zeroNorm_ = Vector(vec[0], vec[1], vec[2]);
  }
}

void TensorGlyphBuilder::computeSinCosTable(bool half)
{
  nv_ = resolution_;
  nu_ = resolution_ + 1;

  // Should only happen when doing half ellipsoids.
  if (half) nv_ /= 2;
  if (nv_ < 2) nv_ = 2;

  double end = half ? M_PI : 2.0 * M_PI;
  tab1_ = SinCosTable(nu_, 0, end);
  tab2_ = SinCosTable(nv_, 0, M_PI);
}

void TensorGlyphBuilder::computeTransforms()
{
  auto eigvecs = t_.getEigenvectors();
  for (auto& v : eigvecs)
    v.normalize();
  GlyphGeomUtility::generateTransforms(center_, GGU::EigenVectorToSCIRunVector(eigvecs[0]),
                                       GGU::EigenVectorToSCIRunVector(eigvecs[1]),
                                       GGU::EigenVectorToSCIRunVector(eigvecs[2]),
                                       trans_, rotate_);
}

void TensorGlyphBuilder::postScaleTransorms()
{
  auto eigvals = t_.getEigenvalues();
  Vector eigvalsVector(eigvals[0], eigvals[1], eigvals[2]);

  trans_.post_scale( Vector(1.0,1.0,1.0) * eigvalsVector);
  rotate_.post_scale(Vector(1.0,1.0,1.0) / eigvalsVector);
}

void TensorGlyphBuilder::generateEllipsoid(GlyphConstructor& constructor, bool half)
{
  const auto prim = Datatypes::SpireIBO::PRIMITIVE::TRIANGLES;
  computeTransforms();
  postScaleTransorms();
  computeSinCosTable(half);
  auto eigvals = t_.getEigenvalues();
  eigvals.inverse();

  for (int v = 0; v < nv_ - 1; ++v)
  {
    double sinPhi[2];
    sinPhi[0] = tab2_.sin(v+1);
    sinPhi[1] = tab2_.sin(v);

    double cosPhi[2];
    cosPhi[0] = tab2_.cos(v+1);
    cosPhi[1] = tab2_.cos(v);

    for (int u = 0; u < nu_; ++u)
    {
      double sinTheta = tab1_.sin(u);
      double cosTheta = tab1_.cos(u);

      // Transorm points and add to points list
      constructor.setOffset(prim);
      for (int i = 0; i < 2; ++i)
      {
        Point point = evaluateEllipsoidPoint(sinPhi[i], cosPhi[i], sinTheta, cosTheta);
        Vector pVector = Vector(trans_ * point);

        Vector normal;
        if(flatTensor_)
        {
          // Avoids recalculating norm vector and prevents vectors with infinite length
          bool first_half = v < nv_/2;
          normal = first_half ? zeroNorm_ : -zeroNorm_;
        }
        else
        {
          normal = rotate_ * Vector(point);
          normal.safe_normalize();
        }

        constructor.addVertex(prim, pVector, normal, color_);
        if (showNormals_)
          constructor.addLine(pVector, pVector + normalDebugScale_ * normal, color_, color_);
      }

      constructor.addIndicesToOffset(prim, 0, 1, 2);
      constructor.addIndicesToOffset(prim, 2, 1, 3);
    }
    constructor.popIndicesNTimes(prim, 6);
  }
}

Point TensorGlyphBuilder::evaluateEllipsoidPoint(double sinPhi, double cosPhi,
                                                 double sinTheta, double cosTheta)
{
  double x, y, z;
  x = sinPhi * sinTheta;
  y = sinPhi * cosTheta;
  z = cosPhi;
  return Point(x, y, z);
}

void TensorGlyphBuilder::generateSuperquadricTensor(GlyphConstructor& constructor, double emphasis)
{
  makeTensorPositive();
  computeTransforms();
  postScaleTransorms();
  computeSinCosTable(false);

  double cl = t_.linearCertainty();
  double cp = t_.planarCertainty();
  bool linear = cl >= cp;

  double pPower = GlyphGeomUtility::spow((1.0 - cp), emphasis);
  double lPower = GlyphGeomUtility::spow((1.0 - cl), emphasis);
  double A = linear ? pPower : lPower;
  double B = linear ? lPower : pPower;

  generateSuperquadricSurfacePrivate(constructor, A, B);
}

void TensorGlyphBuilder::generateSuperquadricSurface(GlyphConstructor& constructor, double A, double B)
{
  computeTransforms();
  postScaleTransorms();
  computeSinCosTable(false);
  generateSuperquadricSurfacePrivate(constructor, A, B);
}

void TensorGlyphBuilder::generateSuperquadricSurfacePrivate(GlyphConstructor& constructor, double A, double B)
{
  const auto prim = Datatypes::SpireIBO::PRIMITIVE::TRIANGLES;
  double cl = t_.linearCertainty();
  double cp = t_.planarCertainty();
  bool linear = cl >= cp;

  for (int v = 0; v < nv_ - 1; ++v)
  {
    double sinPhi[2];
    sinPhi[0] = tab2_.sin(v);
    sinPhi[1] = tab2_.sin(v+1);

    double cosPhi[2];
    cosPhi[0] = tab2_.cos(v);
    cosPhi[1] = tab2_.cos(v+1);

    for (int u = 0; u < nu_; ++u)
    {
      constructor.setOffset(prim);
      double sinTheta = tab1_.sin(u);
      double cosTheta = tab1_.cos(u);

      for(int i = 0; i < 2; ++i)
      {
        // Transorm points and add to points list
        Point p = evaluateSuperquadricPoint(linear, sinPhi[i], cosPhi[i], sinTheta, cosTheta, A, B);
        Vector pVector = Vector(trans_ * p);

        Vector normal;
        if(flatTensor_)
        {
          // Avoids recalculating norm vector and prevents vectors with infinite length
          bool first_half = v < nv_/2;
          normal = first_half ? zeroNorm_ : -zeroNorm_;
        }
        else
        {
          normal = Vector(evaluateSuperquadricNormal(linear, sinPhi[i], cosPhi[i],
                                                    sinTheta, cosTheta, A, B));
          normal = rotate_ * normal;
          normal.safe_normalize();
        }

        constructor.addVertex(prim, pVector, normal, color_);
        if (showNormals_)
          constructor.addLine(pVector, pVector + normalDebugScale_ * normal, color_, color_);
      }

      constructor.addIndicesToOffset(prim, 0, 1, 2);
      constructor.addIndicesToOffset(prim, 2, 1, 3);
    }
  }
  constructor.popIndicesNTimes(prim, 6);
}

Point TensorGlyphBuilder::evaluateSuperquadricNormal(bool linear, double sinPhi, double cosPhi,
                                                    double sinTheta, double cosTheta,
                                                    double A, double B)
{
  if (linear)
    return evaluateSuperquadricNormalLinear(sinPhi, cosPhi, sinTheta, cosTheta, A, B);
  else
    return evaluateSuperquadricNormalPlanar(sinPhi, cosPhi, sinTheta, cosTheta, A, B);
}

// Generate around x-axis
Point TensorGlyphBuilder::evaluateSuperquadricNormalLinear(double sinPhi, double cosPhi,
                                                          double sinTheta, double cosTheta,
                                                          double A, double B)
{
  double x, y, z;
  x =  2.0/B * GGU::spow(cosPhi, 2.0-B);
  y = -2.0/B * GGU::spow(sinPhi, 2.0-B) * GGU::spow(sinTheta, 2.0-A);
  z =  2.0/B * GGU::spow(sinPhi, 2.0-B) * GGU::spow(cosTheta, 2.0-A);
  return Point(x, y, z);
}

// Generate around z-axis
Point TensorGlyphBuilder::evaluateSuperquadricNormalPlanar(double sinPhi, double cosPhi,
                                                          double sinTheta, double cosTheta,
                                                          double A, double B)
{
  double x, y, z;
  x = 2.0/B * GGU::spow(sinPhi, 2.0-B) * GGU::spow(cosTheta, 2.0-A);
  y = 2.0/B * GGU::spow(sinPhi, 2.0-B) * GGU::spow(sinTheta, 2.0-A);
  z = 2.0/B * GGU::spow(cosPhi, 2.0-B);
  return Point(x, y, z);
}

Point TensorGlyphBuilder::evaluateSuperquadricPoint(bool linear, double sinPhi, double cosPhi,
                                                    double sinTheta, double cosTheta,
                                                    double A, double B)
{
  if (linear)
    return evaluateSuperquadricPointLinear(sinPhi, cosPhi, sinTheta, cosTheta, A, B);
  else
    return evaluateSuperquadricPointPlanar(sinPhi, cosPhi, sinTheta, cosTheta, A, B);
}

// Generate around x-axis
Point TensorGlyphBuilder::evaluateSuperquadricPointLinear(double sinPhi, double cosPhi,
                                                          double sinTheta, double cosTheta,
                                                          double A, double B)
{
  double x, y, z;
  x =  GlyphGeomUtility::spow(cosPhi, B);
  y = -GlyphGeomUtility::spow(sinPhi, B) * GlyphGeomUtility::spow(sinTheta, A);
  z =  GlyphGeomUtility::spow(sinPhi, B) * GlyphGeomUtility::spow(cosTheta, A);
  return Point(x, y, z);
}

// Generate around z-axis
Point TensorGlyphBuilder::evaluateSuperquadricPointPlanar(double sinPhi, double cosPhi,
                                                          double sinTheta, double cosTheta,
                                                          double A, double B)
{
  double x, y, z;
  x = GlyphGeomUtility::spow(sinPhi, B) * GlyphGeomUtility::spow(cosTheta, A);
  y = GlyphGeomUtility::spow(sinPhi, B) * GlyphGeomUtility::spow(sinTheta, A);
  z = GlyphGeomUtility::spow(cosPhi, B);
  return Point(x, y, z);
}

void TensorGlyphBuilder::generateBox(GlyphConstructor& constructor)
{
  computeTransforms();

  std::vector<Vector> box_points = generateBoxPoints();
  std::vector<Vector> normals = rotate_.get_rotation_vectors();
  if(flatTensor_)
    for(int d = 0; d < DIMENSIONS_; ++d)
      normals[d] = zeroNorm_;

  generateBoxSide(constructor, box_points[5], box_points[4], box_points[7], box_points[6],  normals[0]);
  generateBoxSide(constructor, box_points[7], box_points[6], box_points[3], box_points[2],  normals[1]);
  generateBoxSide(constructor, box_points[1], box_points[5], box_points[3], box_points[7],  normals[2]);
  generateBoxSide(constructor, box_points[3], box_points[2], box_points[1], box_points[0], -normals[0]);
  generateBoxSide(constructor, box_points[1], box_points[0], box_points[5], box_points[4], -normals[1]);
  generateBoxSide(constructor, box_points[2], box_points[6], box_points[0], box_points[4], -normals[2]);
}

void TensorGlyphBuilder::generateBoxSide(GlyphConstructor& constructor, const Vector& p1, const Vector& p2, const Vector& p3,
                                         const Vector& p4, const Vector& normal)
{
  const auto prim = Datatypes::SpireIBO::PRIMITIVE::TRIANGLES;
  constructor.setOffset(prim);
  constructor.addVertex(prim, p1, normal, color_);
  constructor.addVertex(prim, p2, normal, color_);
  constructor.addVertex(prim, p3, normal, color_);
  constructor.addVertex(prim, p4, normal, color_);
  constructor.addIndicesToOffset(prim, 2, 0, 3);
  constructor.addIndicesToOffset(prim, 1, 3, 0);
  if (showNormals_)
    for (auto& p : {p1, p2, p3, p4})
      constructor.addLine(p, p + normalDebugScale_ * normal, color_, color_);
}

std::vector<Vector> TensorGlyphBuilder::generateBoxPoints()
{
  auto eigvals = t_.getEigenvalues();
  std::vector<Vector> boxPoints;

  for(int x : {-1, 1})
    for(int y : {-1, 1})
      for(int z : {-1, 1})
        boxPoints.emplace_back(trans_ * Point(x * eigvals[0], y * eigvals[1], z * eigvals[2]));

  return boxPoints;
}

