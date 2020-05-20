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

TensorGlyphBuilder::TensorGlyphBuilder(const Tensor& t, const Point& center)
{
  t_ = t;
  center_ = Point(center);
  trans_ = Transform();
  rotate_ = Transform();
}

void TensorGlyphBuilder::setTensor(const Tensor& t)
{
  t_ = t;
}


Tensor TensorGlyphBuilder::getTensor() const
{
  return t_;
}

void TensorGlyphBuilder::scaleTensor(double scale)
{
  t_ = t_ * scale;
}

std::vector<Vector> TensorGlyphBuilder::getEigenVectors()
{
  std::vector<Vector> eigvecs(DIMENSIONS_);
  t_.get_eigenvectors(eigvecs[0], eigvecs[1], eigvecs[2]);
  return eigvecs;
}

std::vector<double> TensorGlyphBuilder::getEigenValues()
{
  std::vector<double> eigvals(DIMENSIONS_);
  t_.get_eigenvalues(eigvals[0], eigvals[1], eigvals[2]);
  return eigvals;
}

void TensorGlyphBuilder::normalizeTensor()
{
  auto eigvecs = getEigenVectors();
  for (auto& v : eigvecs)
    v.normalize();

  auto normEigvals = t_.normalized_eigvals();
  t_.set_outside_eigens(eigvecs[0], eigvecs[1], eigvecs[2],
                        normEigvals[0], normEigvals[1], normEigvals[2]);
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

void TensorGlyphBuilder::makeTensorPositive(bool reorder, bool makeGlyph)
{
  static const double zeroThreshold = 0.000001;

  auto eigvals = getEigenValues();
  auto eigvecs = getEigenVectors();
  for (auto& e : eigvals)
  {
    e = fabs(e);
    if(e <= zeroThreshold)
      e = 0;
  }

  // These are exactly zero after thresholding
  flatTensor_ = eigvals[0] == 0 || eigvals[1] == 0 || eigvals[2] == 0;

  if (makeGlyph)
  {
    auto cross = Cross(eigvecs[0], eigvecs[1]);
    if (Dot(cross, eigvecs[2]) < 2e-12)
      eigvecs[2] = cross;
  }

  for (int d = 0; d < DIMENSIONS_; ++d)
    if (eigvals[d] == 0)
    {
      zeroNorm_ = Cross(eigvecs[(d+1) % DIMENSIONS_], eigvecs[(d+2) % DIMENSIONS_]).normal();
      eigvecs[d] = zeroNorm_ * eigvals[d];
      break;
    }

  t_.set_outside_eigens(eigvecs[0], eigvecs[1], eigvecs[2], eigvals[0], eigvals[1], eigvals[2]);
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
  auto eigvecs = getEigenVectors();
  for (auto& v : eigvecs)
    v.normalize();
  GlyphGeomUtility::generateTransforms(center_, eigvecs[0], eigvecs[1], eigvecs[2], trans_, rotate_);
}

void TensorGlyphBuilder::postScaleTransorms()
{
  auto eigvals = getEigenValues();
  Vector eigvalsVector(eigvals[0], eigvals[1], eigvals[2]);

  trans_.post_scale(eigvalsVector);
}

void TensorGlyphBuilder::generateEllipsoid(GlyphConstructor& constructor, bool half)
{
  makeTensorPositive(true);
  computeTransforms();
  postScaleTransorms();
  computeSinCosTable(half);

  auto eigvals = getEigenValues();
  Vector eigvalsVector(eigvals[0], eigvals[1], eigvals[2]);
  Transform rotateThenInvScale = rotate_;
  rotateThenInvScale.post_scale(Vector(1.0,1.0,1.0) / eigvalsVector);

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
      EllipsoidPointParams params;
      params.sinTheta = tab1_.sin(u);
      params.cosTheta = tab1_.cos(u);

      // Transorm points and add to points list
      constructor.setOffset();
      for (int i = 0; i < 2; ++i)
      {
        params.sinPhi = sinPhi[i];
        params.cosPhi = cosPhi[i];
        Point point = evaluateEllipsoidPoint(params);
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
          normal = rotateThenInvScale * Vector(point);
          normal.safe_normalize();
        }

        constructor.addVertex(pVector, normal, color_);
      }

      constructor.addIndicesToOffset(0, 1, 2);
      constructor.addIndicesToOffset(2, 1, 3);
    }
    constructor.popIndicesNTimes(6);
  }
}

Point TensorGlyphBuilder::evaluateEllipsoidPoint(EllipsoidPointParams& params)
{
  double x, y, z;
  x = params.sinPhi * params.sinTheta;
  y = params.sinPhi * params.cosTheta;
  z = params.cosPhi;
  return Point(x, y, z);
}

bool TensorGlyphBuilder::isLinear()
{
  cl_ = t_.linearCertainty();
  cp_ = t_.planarCertainty();
  return cl_ >= cp_;
}

void TensorGlyphBuilder::computeAAndB(double emphasis)
{
  bool linear = isLinear();
  double pPower = GlyphGeomUtility::spow((1.0 - cp_), emphasis);
  double lPower = GlyphGeomUtility::spow((1.0 - cl_), emphasis);
  A_ = linear ? pPower : lPower;
  B_ = linear ? lPower : pPower;
}

double TensorGlyphBuilder::getA()
{
  return A_;
}

double TensorGlyphBuilder::getB()
{
  return B_;
}

double TensorGlyphBuilder::computeSinPhi(int v)
{
  return tab2_.sin(v);
}

double TensorGlyphBuilder::computeCosPhi(int v)
{
  return tab2_.cos(v);
}

double TensorGlyphBuilder::computeSinTheta(int u)
{
  return tab1_.sin(u);
}

double TensorGlyphBuilder::computeCosTheta(int u)
{
  return tab1_.cos(u);
}

void TensorGlyphBuilder::generateSuperquadricTensor(GlyphConstructor& constructor, double emphasis)
{
  makeTensorPositive(true);
  auto eigvals = getEigenValues();
  auto eigvecs = getEigenVectors();

  computeTransforms();
  postScaleTransorms();
  computeSinCosTable(false);

  bool linear = isLinear();
  computeAAndB(emphasis);
  SuperquadricPointParams params;
  params.A = A_;
  params.B = B_;

  SuperquadricPointParams normalParams;
  normalParams.A = 2.0 - A_;
  normalParams.B = 2.0 - B_;

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
      constructor.setOffset();
      params.sinTheta = normalParams.sinTheta = tab1_.sin(u);
      params.cosTheta = normalParams.cosTheta = tab1_.cos(u);

      for(int i = 0; i < 2; ++i)
      {
        params.sinPhi = normalParams.sinPhi = sinPhi[i];
        params.cosPhi = normalParams.cosPhi = cosPhi[i];
        // Transorm points and add to points list
        Point p = evaluateSuperquadricPoint(linear, params);
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
          normal = Vector(evaluateSuperquadricPoint(linear, normalParams));
          normal = rotate_ * normal;
          normal.safe_normalize();
        }

        constructor.addVertex(pVector, normal, color_);
      }

      constructor.addIndicesToOffset(0, 1, 2);
      constructor.addIndicesToOffset(2, 1, 3);
    }
  }
  constructor.popIndicesNTimes(6);
}

Point TensorGlyphBuilder::evaluateSuperquadricPoint(bool linear, const SuperquadricPointParams& params)
{
  if (linear)
    return evaluateSuperquadricPointLinear(params);
  else
    return evaluateSuperquadricPointPlanar(params);
}

// Generate around x-axis
Point TensorGlyphBuilder::evaluateSuperquadricPointLinear(const SuperquadricPointParams& params)
{
  double x, y, z;
  x =  GlyphGeomUtility::spow(params.cosPhi, params.B);
  y = -GlyphGeomUtility::spow(params.sinPhi, params.B) * GlyphGeomUtility::spow(params.sinTheta, params.A);
  z =  GlyphGeomUtility::spow(params.sinPhi, params.B) * GlyphGeomUtility::spow(params.cosTheta, params.A);
  return Point(x, y, z);
}

// Generate around z-axis
Point TensorGlyphBuilder::evaluateSuperquadricPointPlanar(const SuperquadricPointParams& params)
{
  double x, y, z;
  x = GlyphGeomUtility::spow(params.sinPhi, params.B) * GlyphGeomUtility::spow(params.cosTheta, params.A);
  y = GlyphGeomUtility::spow(params.sinPhi, params.B) * GlyphGeomUtility::spow(params.sinTheta, params.A);
  z = GlyphGeomUtility::spow(params.cosPhi, params.B);
  return Point(x, y, z);
}

void TensorGlyphBuilder::generateBox(GlyphConstructor& constructor)
{
  computeTransforms();

  std::vector<Vector> box_points = generateBoxPoints();
  std::vector<Vector> normals = rotate_.get_rotation();
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
  constructor.setOffset();
  constructor.addVertex(p1, normal, color_);
  constructor.addVertex(p2, normal, color_);
  constructor.addVertex(p3, normal, color_);
  constructor.addVertex(p4, normal, color_);
  constructor.addIndicesToOffset(2, 0, 3);
  constructor.addIndicesToOffset(1, 3, 0);
}

std::vector<Vector> TensorGlyphBuilder::generateBoxPoints()
{
  auto eigvals = getEigenValues();
  std::vector<Vector> boxPoints;

  for(int x : {-1, 1})
    for(int y : {-1, 1})
      for(int z : {-1, 1})
        boxPoints.emplace_back(trans_ * Point(x * eigvals[0], y * eigvals[1], z * eigvals[2]));

  return boxPoints;
}

Transform TensorGlyphBuilder::getTrans()
{
  return trans_;
}

Transform TensorGlyphBuilder::getRotate()
{
  return rotate_;
}

Transform TensorGlyphBuilder::getScale()
{
  Transform scale = Transform();
  auto eigvals = getEigenValues();
  for (int i = 0; i < eigvals.size(); ++i)
    scale.set_mat_val(i, i, eigvals[i]);
  return scale;
}
