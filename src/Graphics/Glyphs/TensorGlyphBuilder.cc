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

UncertaintyTensorOffsetSurfaceBuilder::UncertaintyTensorOffsetSurfaceBuilder(
  const Core::Datatypes::Dyadic3DTensor& t, const Core::Geometry::Point& center, double emphasis,
  const Eigen::Matrix<double, 6, 6>& covarianceMatrix)
    : TensorGlyphBuilder(t, center), emphasis_(emphasis), covarianceMatrix_(covarianceMatrix)
{
  h_ = 0.000001;// * t.frobeniusNorm();
  hHalf_ = h_ * 0.5;
}


/* Tim Gerrits' Uncertainty Tensor Glyphs
 https://onlinelibrary.wiley.com/doi/abs/10.1111/cgf.13692 */
void UncertaintyTensorOffsetSurfaceBuilder::generateOffsetSurface(GlyphConstructor& constructor)
{
  Vector centerVector(center_);

  t_.makePositive();
  t_.setDescendingRHSOrder();
  computeTransforms();
  postScaleTransforms();
  computeSinCosTable(false);

  std::vector<Eigen::Vector3d> eigvecs = t_.getEigenvectors();
  MandelVector tMandel = t_.mandel();

  Eigen::Matrix3d rotate = t_.getEigenvectorsAsMatrix();
  Eigen::Matrix3d rotateTranspose = rotate.transpose();
  Eigen::Vector3d eigvals = t_.getEigenvalues();
  Eigen::Vector3d eigvalsInv = eigvals.asDiagonal().inverse().diagonal();
  Eigen::Matrix3d scale = eigvals.asDiagonal();
  Eigen::Matrix3d scaleInv = eigvalsInv.asDiagonal();

  const auto scaleThenRotate = rotate * scale;
  const auto undoScaleAndRotate = scaleInv * rotateTranspose;

  double cl = t_.linearCertainty();
  double cp = t_.planarCertainty();
  bool linear = cl >= cp;

  double pPower = GlyphGeomUtility::spow((1.0 - cp), emphasis_);
  double lPower = GlyphGeomUtility::spow((1.0 - cl), emphasis_);
  double A = linear ? pPower : lPower;
  double B = linear ? lPower : pPower;

  SuperquadricPointParams params;
  params.A = A;
  params.B = B;

  SuperquadricPointParams normalParams;
  normalParams.A = 2.0 - params.A;
  normalParams.B = 2.0 - params.B;

  MandelVector qn;
  DifftValues difftVals;
  precalculateDifftValues(difftVals, tMandel);
  int nv = resolution_;
  int nu = resolution_ + 1;

  for (int v = 0; v < nv - 1; ++v)
  {
    double sinPhi[2] = {tab2_.sin(v), tab2_.sin(v + 1)};
    double cosPhi[2] = {tab2_.cos(v), tab2_.cos(v + 1)};
    for (int u = 0; u < nu; ++u)
    {
      constructor.setOffset(PRIM_);
      params.sinTheta = normalParams.sinTheta = tab1_.sin(u);
      params.cosTheta = normalParams.cosTheta = tab1_.cos(u);
      for (int i = 0; i < 2; ++i)
      {
        params.sinPhi = normalParams.sinPhi = sinPhi[i];
        params.cosPhi = normalParams.cosPhi = cosPhi[i];

        Vector posVector = Vector(evaluateSuperquadricPoint(linear, params));
        Eigen::Vector3d pos = scaleThenRotate * Eigen::Vector3d(posVector.x(), posVector.y(), posVector.z());
        Vector normalVector = evaluateSuperquadricNormal(linear, normalParams);
        Eigen::Vector3d normal = Eigen::Vector3d(normalVector.x(), normalVector.y(), normalVector.z());
        normal = normal.cwiseProduct(eigvalsInv).normalized();

        // Surface Derivative
        Eigen::Vector3d nn;
        Eigen::Vector3d diff(0.0, 0.0, 0.0);
        for (int j = 0; j < 3; ++j)
        {
          diff[j] = hHalf_;
          Eigen::Vector3d newPos = undoScaleAndRotate * (pos + diff);
          double d1 = evaluateSuperquadricImpl(linear, newPos, params.A, params.B);

          newPos = undoScaleAndRotate * (pos - diff);
          double d2 = evaluateSuperquadricImpl(linear, newPos, params.A, params.B);
          diff[j] = 0.0;
          nn(j) = (d1 - d2) / h_;
        }
        MandelVector qn = getQn(difftVals, pos);
        qn /= h_;
        qn /= nn.norm();
        double q = std::sqrt(
            std::abs((qn.transpose().eval() * (covarianceMatrix_ * qn).eval()).eval().value()));

        Eigen::Vector3d n = nn / nn.norm();
        Vector nVector = Vector(n.x(), n.y(), n.z());

        Eigen::Vector3d rotatedNormal = rotate * normal;
        Eigen::Vector3d offsetPos = pos + q * rotatedNormal;
        Vector offsetPosVector = Vector(offsetPos.x(), offsetPos.y(), offsetPos.z());

        constructor.addVertex(PRIM_, offsetPosVector + centerVector, nVector, ColorRGB(1.0, 1.0, 1.0));
      }

      constructor.addIndicesToOffset(PRIM_, 0, 1, 2);
      constructor.addIndicesToOffset(PRIM_, 2, 1, 3);
    }
  }
  constructor.popIndicesNTimes(PRIM_, 6);
}

// Precalculate transformations for getQn()
void UncertaintyTensorOffsetSurfaceBuilder::precalculateDifftValues(
    DifftValues& vals, const MandelVector& t)
{
  MandelVector finiteDiff;
  finiteDiff.fill(0.0);

  for (auto i = 0; i < DEGREES_OF_FREEDOM_; ++i)
  {
    finiteDiff(i) = hHalf_;
    std::vector<double> dist(2);
    for (auto s = 0; s < 2; ++s)
    {
      auto index = 2 * i + s;
      const MandelVector& tMandel =
          (s == 0) ? MandelVector(t + finiteDiff) : MandelVector(t - finiteDiff);

      auto newT = Dyadic3DTensor(tMandel);
      double cl = newT.linearCertainty();
      double cp = newT.planarCertainty();
      vals.linear[index] = cl >= cp;

      double pPower = GlyphGeomUtility::spow((1.0 - cp), emphasis_);
      double lPower = GlyphGeomUtility::spow((1.0 - cl), emphasis_);
      vals.A[index] = vals.linear[index] ? pPower : lPower;
      vals.B[index] = vals.linear[index] ? lPower : pPower;
      Eigen::Matrix3d rotateTranspose = newT.getEigenvectorsAsMatrix().transpose();
      Eigen::Matrix3d scaleInv = newT.getEigenvalues().asDiagonal().inverse();
      vals.undoScaleAndRotate[index] = scaleInv * rotateTranspose;
      vals.normEigvals[index] = newT.getNormalizedEigenvalues();
    }
    finiteDiff(i) = 0.0;
  }
}

void UncertaintyTensorOffsetSurfaceBuilder::scaleCovariance(double scale)
{
  covarianceMatrix_ *= std::pow(scale, 2);
}

// The vector Qn is a mandel vector of the finite difference of the mean tensor under a small perturbation for all 6 degrees of freedom
MandelVector UncertaintyTensorOffsetSurfaceBuilder::getQn(
    const DifftValues& vals, const Eigen::Vector3d& p)
{
  MandelVector qn;
  for (int i = 0; i < DEGREES_OF_FREEDOM_; ++i)
  {
    std::vector<double> dist(2);
    for (int s = 0; s < 2; ++s)
    {
      auto index = 2 * i + s;
      Eigen::Vector3d newP = vals.undoScaleAndRotate[index] * p;
      dist[s] = evaluateSuperquadricImpl(vals.linear[index], newP, vals.A[index], vals.B[index]);
    }
    qn(i) = dist[0] - dist[1];
  }

  return qn;
}

double UncertaintyTensorOffsetSurfaceBuilder::evaluateSuperquadricImpl(
    bool linear, const Eigen::Vector3d& p, double A, double B)
{
  if (linear)
    return evaluateSuperquadricImplLinear(p, A, B);
  else
    return evaluateSuperquadricImplPlanar(p, A, B);
}

// Generate around x-axis
double UncertaintyTensorOffsetSurfaceBuilder::evaluateSuperquadricImplLinear(
    const Eigen::Vector3d& p, double A, double B)
{
  // auto orthoT = Dyadic3DTensor({Eigen::Vector3d(1,0,0),Eigen::Vector3d(0,1,0),Eigen::Vector3d(0,0,1)}, p);
  // auto cl = orthoT.linearCertainty();
  // auto cp = orthoT.planarCertainty();
  // auto AAndB = getAAndB(cl, cp, cl > cp, emphasis_);
  // A = AAndB.first;
  // B = AAndB.second;
  const double twoDivA = 2.0 / A;
  const double twoDivB = 2.0 / B;
  return GGU::spow(
             GGU::spow(std::abs(p.y()), twoDivA) + GGU::spow(std::abs(p.z()), twoDivA), A / B) +
         GGU::spow(std::abs(p.x()), twoDivB) - 1;
}

// Generate around z-axis
double UncertaintyTensorOffsetSurfaceBuilder::evaluateSuperquadricImplPlanar(
    const Eigen::Vector3d& p, double A, double B)
{
  const double twoDivA = 2.0 / A;
  const double twoDivB = 2.0 / B;
  return GGU::spow(
             GGU::spow(std::abs(p.x()), twoDivA) + GGU::spow(std::abs(p.y()), twoDivA), A / B) +
         GGU::spow(std::abs(p.z()), twoDivB) - 1;
}

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

void TensorGlyphBuilder::postScaleTransforms()
{
  auto eigvals = t_.getEigenvalues();
  Vector eigvalsVector(eigvals[0], eigvals[1], eigvals[2]);

  trans_.post_scale( Vector(1.0,1.0,1.0) * eigvalsVector);
  rotate_.post_scale(Vector(1.0,1.0,1.0) / eigvalsVector);
}

void TensorGlyphBuilder::generateEllipsoid(GlyphConstructor& constructor, bool half)
{
  computeTransforms();
  postScaleTransforms();
  computeSinCosTable(half);
  auto eigvals = t_.getEigenvalues();
  eigvals.inverse();
  EllipsoidPointParams params;

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
      params.sinTheta = tab1_.sin(u);
      params.cosTheta = tab1_.cos(u);

      // Transorm points and add to points list
      constructor.setOffset(PRIM_);
      for (int i = 0; i < 2; ++i)
      {
        params.sinPhi = sinPhi[i];
        params.cosPhi = cosPhi[i];
        Point point = evaluateEllipsoidPoint(params);
        Vector posVector = Vector(trans_ * point);

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

        constructor.addVertex(PRIM_, posVector, normal, color_);
        if (showNormals_)
          constructor.addLine(posVector, posVector + normalDebugScale_ * normal, color_, color_);
      }

      constructor.addIndicesToOffset(PRIM_, 0, 1, 2);
      constructor.addIndicesToOffset(PRIM_, 2, 1, 3);
    }
    constructor.popIndicesNTimes(PRIM_, 6);
  }
}

Point TensorGlyphBuilder::evaluateEllipsoidPoint(const EllipsoidPointParams& params)
{
  double x, y, z;
  x = params.sinPhi * params.sinTheta;
  y = params.sinPhi * params.cosTheta;
  z = params.cosPhi;
  return Point(x, y, z);
}

void TensorGlyphBuilder::generateSuperquadricTensor(GlyphConstructor& constructor, double emphasis)
{
  makeTensorPositive();
  computeTransforms();
  postScaleTransforms();
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
  postScaleTransforms();
  computeSinCosTable(false);
  generateSuperquadricSurfacePrivate(constructor, A, B);
}

void TensorGlyphBuilder::generateSuperquadricSurfacePrivate(GlyphConstructor& constructor, double A, double B)
{
  double cl = t_.linearCertainty();
  double cp = t_.planarCertainty();
  bool linear = cl >= cp;

  SuperquadricPointParams params;
  params.A = A;
  params.B = B;
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
      constructor.setOffset(PRIM_);
      params.sinTheta = tab1_.sin(u);
      params.cosTheta = tab1_.cos(u);

      for(int i = 0; i < 2; ++i)
      {
        params.sinPhi = sinPhi[i];
        params.cosPhi = cosPhi[i];
        // Transorm points and add to points list
        Point pos = evaluateSuperquadricPoint(linear, params);
        Vector posVector = Vector(trans_ * pos);

        Vector normal;
        if(flatTensor_)
        {
          // Avoids recalculating norm vector and prevents vectors with infinite length
          bool first_half = v < nv_/2;
          normal = first_half ? zeroNorm_ : -zeroNorm_;
        }
        else
        {
          normal = Vector(evaluateSuperquadricNormal(linear, params));
          normal = rotate_ * normal;
          normal.safe_normalize();
        }

        constructor.addVertex(PRIM_, posVector, normal, color_);
        if (showNormals_)
          constructor.addLine(posVector, posVector + normalDebugScale_ * normal, color_, color_);
      }

      constructor.addIndicesToOffset(PRIM_, 0, 1, 2);
      constructor.addIndicesToOffset(PRIM_, 2, 1, 3);
    }
  }
  constructor.popIndicesNTimes(PRIM_, 6);
}

Vector TensorGlyphBuilder::evaluateSuperquadricNormal(bool linear, const SuperquadricPointParams& params)
{
  if (linear)
    return evaluateSuperquadricNormalLinear(params);
  else
    return evaluateSuperquadricNormalPlanar(params);
}

// Generate around x-axis
Vector TensorGlyphBuilder::evaluateSuperquadricNormalLinear(const SuperquadricPointParams& params)
{
  double x, y, z;
  x =  2.0/params.B * GGU::spow(params.cosPhi, 2.0-params.B);
  y = -2.0/params.B * GGU::spow(params.sinPhi, 2.0-params.B) * GGU::spow(params.sinTheta, 2.0-params.A);
  z =  2.0/params.B * GGU::spow(params.sinPhi, 2.0-params.B) * GGU::spow(params.cosTheta, 2.0-params.A);
  return Vector(x, y, z);
}

// Generate around z-axis
Vector TensorGlyphBuilder::evaluateSuperquadricNormalPlanar(const SuperquadricPointParams& params)
{
  double x, y, z;
  x = 2.0/params.B * GGU::spow(params.sinPhi, 2.0-params.B) * GGU::spow(params.cosTheta, 2.0-params.A);
  y = 2.0/params.B * GGU::spow(params.sinPhi, 2.0-params.B) * GGU::spow(params.sinTheta, 2.0-params.A);
  z = 2.0/params.B * GGU::spow(params.cosPhi, 2.0-params.B);
  return Vector(x, y, z);
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
  constructor.setOffset(PRIM_);
  constructor.addVertex(PRIM_, p1, normal, color_);
  constructor.addVertex(PRIM_, p2, normal, color_);
  constructor.addVertex(PRIM_, p3, normal, color_);
  constructor.addVertex(PRIM_, p4, normal, color_);
  constructor.addIndicesToOffset(PRIM_, 2, 0, 3);
  constructor.addIndicesToOffset(PRIM_, 1, 3, 0);
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

