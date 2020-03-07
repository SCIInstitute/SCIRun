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
#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace Graphics;
using namespace Core::Geometry;
using namespace Core::Datatypes;

TensorGlyphBuilder::TensorGlyphBuilder(const Tensor& t, const Point& center)
{
  t_ = t;
  center_ = Point(center);
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

  auto norm = t_.euclidean_norm();
  t_.set_outside_eigens(eigvecs[0] * norm[0], eigvecs[1] * norm[1], eigvecs[2] * norm[2],
                        norm[0], norm[1], norm[2]);
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

  if (flatTensor_)
    reorderTensorValues(eigvecs, eigvals);

  for (int d = 0; d < DIMENSIONS_; ++d)
    if (eigvals[d] == 0)
    {
      zeroNorm_ = Cross(eigvecs[(d+1) % DIMENSIONS_], eigvecs[(d+2) % DIMENSIONS_]).normal();
      eigvecs[d] = zeroNorm_ * eigvals[d];
      break;
    }

  t_.set_outside_eigens(eigvecs[0], eigvecs[1], eigvecs[2],
                        eigvals[0], eigvals[1], eigvals[2]);
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

  trans_.post_scale( Vector(1.0,1.0,1.0) * eigvalsVector);
  rotate_.post_scale(Vector(1.0,1.0,1.0) / eigvalsVector);
}

void TensorGlyphBuilder::generateEllipsoid(GlpyhConstructor& constructor, bool half)
{
  computeTransforms();
  postScaleTransorms();
  computeSinCosTable(half);

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
      uint32_t offset = static_cast<uint32_t>(geomData.numVBOElements);
      double sinTheta = tab1_.sin(u);
      double cosTheta = tab1_.cos(u);

      // Transorm points and add to points list
      for (int i = 0; i < 2; ++i)
      {
        Point point = evaluateEllipsoidPoint(sinPhi[i], cosPhi[i], sinTheta, cosTheta);
        Vector pVector = Vector(trans_ * point);
        geomData.points.push_back(pVector);

        Vector normal = Vector(point);
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
        geomData.normals.push_back(normal);
        geomData.colors.push_back(color_);
        geomData.numVBOElements++;
      }

      geomData.indices.push_back(0 + offset);
      geomData.indices.push_back(1 + offset);
      geomData.indices.push_back(2 + offset);
      geomData.indices.push_back(2 + offset);
      geomData.indices.push_back(1 + offset);
      geomData.indices.push_back(3 + offset);
    }
    for(int jj = 0; jj < 6; jj++) geomData.indices.pop_back();
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

void TensorGlyphBuilder::generateSuperquadricTensor(GlpyhConstructor& constructor, double emphasis)
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

  double normalA = 2.0-A;
  double normalB = 2.0-B;

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
      uint32_t offset = static_cast<uint32_t>(geomData.numVBOElements);
      double sinTheta = tab1_.sin(u);
      double cosTheta = tab1_.cos(u);

      for(int i = 0; i < 2; ++i)
      {
        // Transorm points and add to points list
        Point p = evaluateSuperquadricPoint(linear, sinPhi[i], cosPhi[i], sinTheta, cosTheta, A, B);

        Vector pVector = Vector(trans_ * p);
        geomData.points.push_back(pVector);

        Vector normal;
        if(flatTensor_)
        {
          // Avoids recalculating norm vector and prevents vectors with infinite length
          bool first_half = v < nv_/2;
          normal = first_half ? zeroNorm_ : -zeroNorm_;
        }
        else
        {
          normal = Vector(evaluateSuperquadricPoint(linear, sinPhi[i], cosPhi[i],
                                                    sinTheta, cosTheta, normalA, normalB));
          normal = rotate_ * normal;
          normal.safe_normalize();
        }
        geomData.normals.push_back(normal);
        geomData.colors.push_back(color_);
        geomData.numVBOElements++;
      }

      geomData.indices.push_back(0 + offset);
      geomData.indices.push_back(1 + offset);
      geomData.indices.push_back(2 + offset);
      geomData.indices.push_back(2 + offset);
      geomData.indices.push_back(1 + offset);
      geomData.indices.push_back(3 + offset);
    }
  }
  for(int jj = 0; jj < 6; jj++) geomData.indices.pop_back();
}

Point TensorGlyphBuilder::evaluateSuperquadricPoint(bool linear, double sinPhi, double cosPhi,
                                                    double sinTheta, double cosTheta,
                                                    double A, double B)
{
  Point p;
  if (linear)
    p = evaluateSuperquadricPointLinear(sinPhi, cosPhi, sinTheta, cosTheta, A, B);
  else
    p = evaluateSuperquadricPointPlanar(sinPhi, cosPhi, sinTheta, cosTheta, A, B);
  return p;
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

void TensorGlyphBuilder::generateBox(GlpyhConstructor& constructor)
{
  computeTransforms();

  std::vector<Vector> box_points = generateBoxPoints();
  std::vector<Vector> normals = rotate_.get_column_vectors();
  if(flatTensor_)
    for(int d = 0; d < DIMENSIONS_; ++d)
      normals[d] = zeroNorm_;

  generateBoxSide(geomData, box_points[5], box_points[4], box_points[7], box_points[6],  normals[0]);
  generateBoxSide(geomData, box_points[7], box_points[6], box_points[3], box_points[2],  normals[1]);
  generateBoxSide(geomData, box_points[1], box_points[5], box_points[3], box_points[7],  normals[2]);
  generateBoxSide(geomData, box_points[3], box_points[2], box_points[1], box_points[0], -normals[0]);
  generateBoxSide(geomData, box_points[1], box_points[0], box_points[5], box_points[4], -normals[1]);
  generateBoxSide(geomData, box_points[2], box_points[6], box_points[0], box_points[4], -normals[2]);
}

void TensorGlyphBuilder::generateBoxSide(GlpyhConstructor& constructor, const Vector& p1, const Vector& p2,
                                         const Vector& p3, const Vector& p4, const Vector& normal)
{
  geomData.points.push_back(p1);
  geomData.points.push_back(p2);
  geomData.points.push_back(p3);
  geomData.points.push_back(p4);

  for(int p = 0; p < BOX_FACE_POINTS_; p++)
  {
    geomData.normals.push_back(normal);
    geomData.colors.push_back(color_);
  }

  size_t offset = static_cast<size_t>(geomData.numVBOElements);
  geomData.indices.push_back(offset + 2);
  geomData.indices.push_back(offset);
  geomData.indices.push_back(offset + 3);
  geomData.indices.push_back(offset + 1);
  geomData.indices.push_back(offset + 3);
  geomData.indices.push_back(offset);

  geomData.numVBOElements += BOX_FACE_POINTS_;
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

