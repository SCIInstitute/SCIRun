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


#include <Core/Algorithms/Math/ComputeTensorUncertaintyAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Dyadic3DTensor.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/GeometryPrimitives/Point.h>
#include <boost/tuple/tuple.hpp>
#include <unsupported/Eigen/MatrixFunctions>

using namespace SCIRun;
using namespace Core;
using namespace Core::Datatypes;
using namespace Core::Algorithms;
using namespace Core::Algorithms::Math;
using namespace Core::Geometry;

AlgorithmOutputName ComputeTensorUncertaintyAlgorithm::MeanTensorField("MeanTensorField");
AlgorithmOutputName ComputeTensorUncertaintyAlgorithm::CovarianceMatrix("CovarianceMatrix");

ALGORITHM_PARAMETER_DEF(Math, MeanInvariantMethod);
ALGORITHM_PARAMETER_DEF(Math, MeanOrientationMethod);

// TODO move helpers
Dyadic3DTensor scirunTensorToEigenTensor(const Geometry::Tensor& t)
{
  Dyadic3DTensor newTensor(t.xx(), t.xy(), t.xz(), t.yy(), t.yz(), t.zz());
  return newTensor;
}

Tensor eigenTensorToScirunTensor(const Dyadic3DTensor& t)
{
  return Tensor(t(0, 0), t(1, 0), t(2, 0), t(1, 1), t(2, 1), t(2, 2));
}

// TODO move to separate algo
enum FieldDataType
{ node, edge, face, cell };

class ComputeTensorUncertaintyAlgorithmImpl
{
public:
  ComputeTensorUncertaintyAlgorithmImpl();
  void run(const FieldList& fields);
  void setInvariantMethod(AlgoOption method);
  void setOrientationMethod(AlgoOption method);
  void getPoints(const FieldList& fields);
  void getPointsForFields(FieldHandle field, std::vector<int>& indices, std::vector<Point>& points);
  FieldHandle getMeanTensors() const;
  DenseMatrixHandle getCovarianceMatrices() const;
private:
  size_t fieldCount_ = 0;
  size_t fieldSize_ = 0;
  std::vector<std::vector<Point>> points_;
  std::vector<std::vector<int>> indices_;
  std::vector<std::vector<Dyadic3DTensor>> tensors_;
  std::vector<Dyadic3DTensor> meanTensors_;
  std::vector<DyadicTensor<6>> covarianceMatrices_;
  AlgoOption invariantMethod_;
  AlgoOption orientationMethod_;

  void verifyData(const FieldList& fields);
  void getTensors(const FieldList& fields);
  void computeCovarianceMatrices();
  void computeMeanTensors();
  Dyadic3DTensor computeMeanMatrixAverage(int index) const;
  Dyadic3DTensor computeMeanLinearInvariant(int t) const;
  Dyadic3DTensor computeMeanLogEuclidean(int t) const;
  void computeMeanTensorsSameMethod();
  void computeMeanTensorsDifferentMethod();
};

void ComputeTensorUncertaintyAlgorithmImpl::setInvariantMethod(AlgoOption method)
{
  invariantMethod_ = method;
}

void ComputeTensorUncertaintyAlgorithmImpl::setOrientationMethod(AlgoOption method)
{
  orientationMethod_ = method;
}

FieldHandle ComputeTensorUncertaintyAlgorithmImpl::getMeanTensors() const
{
  FieldInformation ofinfo("PointCloudMesh", 0, "Tensor");
  auto ofield = CreateField(ofinfo);
  auto mesh = ofield->vmesh();
  auto field = ofield->vfield();

  std::vector<VMesh::index_type> meshIndices(fieldSize_);
  for (size_t i = 0; i < fieldSize_; ++i)
    meshIndices[i] = mesh->add_point(points_[0][i]);

  field->resize_fdata();

  for (size_t i = 0; i < fieldSize_; ++i)
    field->set_value(eigenTensorToScirunTensor(meanTensors_[i]), meshIndices[i]);

  return ofield;
}

void ComputeTensorUncertaintyAlgorithmImpl::computeMeanTensorsSameMethod()
{
  if (orientationMethod_.option_ == "Matrix Average")
  {
    for (size_t t = 0; t < fieldSize_; ++t)
      meanTensors_[t] = computeMeanMatrixAverage(t);
  }
  else if (orientationMethod_.option_ == "Log-Euclidean")
  {
    for (size_t t = 0; t < fieldSize_; ++t)
      meanTensors_[t] = computeMeanLogEuclidean(t);
  }
}

void ComputeTensorUncertaintyAlgorithmImpl::computeMeanTensorsDifferentMethod()
{
  std::vector<Dyadic3DTensor> orientationTensors(fieldSize_);
  std::vector<Dyadic3DTensor> invariantTensors(fieldSize_);
  if (orientationMethod_.option_ == "Matrix Average")
  {
    for (size_t t = 0; t < fieldSize_; ++t)
      orientationTensors[t] = computeMeanMatrixAverage(t);
  }
  else if (orientationMethod_.option_ == "Log-Euclidean")
  {
    for (size_t t = 0; t < fieldSize_; ++t)
      orientationTensors[t] = computeMeanLogEuclidean(t);
  }

  if (invariantMethod_.option_ == "Matrix Average")
  {
    for (size_t t = 0; t < fieldSize_; ++t)
      invariantTensors[t] = computeMeanMatrixAverage(t);
  }
  else if (invariantMethod_.option_ == "Log-Euclidean")
  {
    for (size_t t = 0; t < fieldSize_; ++t)
      invariantTensors[t] = computeMeanLogEuclidean(t);
  }
  else if (invariantMethod_.option_ == "Linear Invariant")
  {
    for (size_t t = 0; t < fieldSize_; ++t)
      invariantTensors[t] = computeMeanLinearInvariant(t);
  }

  for (size_t t = 0; t < fieldSize_; ++t)
  {
    invariantTensors[t].setDescendingRHSOrder();
    orientationTensors[t].setDescendingRHSOrder();
    meanTensors_[t] = Dyadic3DTensor(orientationTensors[t].getEigenvectors(),
                                     invariantTensors[t].getEigenvalues());
    meanTensors_[t].setDescendingRHSOrder();
  }
}

void ComputeTensorUncertaintyAlgorithmImpl::computeMeanTensors()
{
  meanTensors_ = std::vector<Dyadic3DTensor>(fieldSize_);
  if (invariantMethod_.option_ == orientationMethod_.option_)
    computeMeanTensorsSameMethod();
  else
    computeMeanTensorsDifferentMethod();
}

Dyadic3DTensor ComputeTensorUncertaintyAlgorithmImpl::computeMeanMatrixAverage(int t) const
{
  Dyadic3DTensor sum = Dyadic3DTensor();
  for (size_t f = 0; f < fieldCount_; ++f) {
    sum += tensors_[f][t];
  }
  sum = sum / (double)fieldCount_;

  return sum;
}

// Mean calculation using Linear Invariant interpolation
Dyadic3DTensor ComputeTensorUncertaintyAlgorithmImpl::computeMeanLinearInvariant(int t) const
{
  const static double oneThird = 1.0 / 3.0;
  const static double sqrtThreeHalves = sqrt(1.5);
  const static double threeSqrtSix = 3.0 * sqrt(6.0);

  const static Dyadic3DTensor identity = Dyadic3DTensor(Eigen::Vector3d(1, 0, 0),
                                                        Eigen::Vector3d(0, 1, 0),
                                                        Eigen::Vector3d(0, 0, 1));
  const static Dyadic3DTensor identityThird = oneThird * identity;

  double K1 = 0.0;
  double R2 = 0.0;
  double R3 = 0.0;

  for (size_t f = 0; f < fieldCount_; ++f)
  {
    const double trace = tensors_[f][t].trace();
    Dyadic3DTensor trThird = trace * identityThird;
    K1 += trace;

    const double fro = tensors_[f][t].frobeniusNorm();
    Dyadic3DTensor anisotropicDeviation = tensors_[f][t] - trThird;
    anisotropicDeviation.setDescendingRHSOrder();
    const double anisotropicDeviationFro = anisotropicDeviation.frobeniusNorm();

    R2 += sqrtThreeHalves * anisotropicDeviationFro / fro;
    R3 += threeSqrtSix * (anisotropicDeviation / anisotropicDeviationFro).asMatrix().determinant();
  }

  // Equally weight all of the coeffecients
  K1 /= static_cast<double>(fieldCount_);
  R2 /= static_cast<double>(fieldCount_);
  R3 /= static_cast<double>(fieldCount_);

  // Clamp to avoid nan results with acos
  if (R3 > 1.0) R3 = 1.0;
  if (R3 < -1.0) R3 = -1.0;

  
  const double arccosR3 = std::acos(R3);
  Eigen::Vector3d eigvals;
  const double x = oneThird * K1;
  const double y = (2.0 * K1 * R2) / (3.0 * sqrt(3.0 - 2.0 * std::pow(R2, 2.0)));
  eigvals(0) = x + y * std::cos(arccosR3 * oneThird);
  eigvals(1) = x + y * std::cos((arccosR3 - 2.0 * M_PI) * oneThird);
  eigvals(2) = x + y * std::cos((arccosR3 + 2.0 * M_PI) * oneThird);

  // Using axis aligned orientation because this mean method is only for invariants
  Dyadic3DTensor ret = Dyadic3DTensor({Eigen::Vector3d({1,0,0}), Eigen::Vector3d({0,1,0}), Eigen::Vector3d({0,0,1})}, eigvals);
  return ret;
}

Dyadic3DTensor ComputeTensorUncertaintyAlgorithmImpl::computeMeanLogEuclidean(int t) const
{
  Eigen::Matrix3d sum = Eigen::Matrix3d::Zero();
  for (size_t f = 0; f < fieldCount_; ++f)
  {
    Dyadic3DTensor tensor = tensors_[f][t];
    tensor.setDescendingRHSOrder();
    Eigen::Vector3d eigvalsLog = tensor.getEigenvalues();
    Eigen::Matrix3d rotation = tensor.getEigenvectorsAsMatrix();
    for (size_t i = 0; i < eigvalsLog.size(); ++i)
      eigvalsLog[i] = std::log(eigvalsLog[i]);
    sum += rotation * eigvalsLog.asDiagonal() * rotation.transpose();
  }
  sum /= (double)fieldCount_;
  Eigen::Matrix3d mean = sum.exp();
  Dyadic3DTensor ret = Dyadic3DTensor(mean);
  return ret;
}

ComputeTensorUncertaintyAlgorithmImpl::ComputeTensorUncertaintyAlgorithmImpl()
{}

void ComputeTensorUncertaintyAlgorithmImpl::computeCovarianceMatrices()
{
  covarianceMatrices_ = std::vector<DyadicTensor<6>>(fieldSize_);

  Eigen::Matrix<double, 6, 6> covarianceMat;
  for (size_t t = 0; t < fieldSize_; ++t)
  {
    covarianceMat.fill(0.0);
    for (size_t f = 0; f < fieldCount_; ++f)
    {
      Eigen::Matrix<double, 6, 1> diffTensor = (tensors_[f][t] - meanTensors_[t]).mandel();
      covarianceMat += diffTensor * diffTensor.transpose();
    }
    covarianceMat /= static_cast<double>(fieldCount_);
    covarianceMatrices_[t] = DyadicTensor<6>(covarianceMat);
  }
}

void ComputeTensorUncertaintyAlgorithmImpl::getPointsForFields(
    FieldHandle field, std::vector<int>& indices, std::vector<Point>& points)
{
  // Collect indices and points from facades
  FieldDataType fieldLocation;
  FieldInformation finfo(field);
  if (finfo.is_point() || finfo.is_linear())
    fieldLocation = FieldDataType::node;
  else if (finfo.is_line())
    fieldLocation = FieldDataType::edge;
  else if (finfo.is_surface())
    fieldLocation = FieldDataType::face;
  else
    fieldLocation = FieldDataType::cell;

  auto mesh = field->vmesh();
  auto primaryFacade = field->mesh()->getFacade();
  switch (fieldLocation)
  {
  case FieldDataType::node:
    for (const auto& node : primaryFacade->nodes())
    {
      indices.push_back(node.index());
      Point p;
      mesh->get_center(p, node.index());
      points.push_back(p);
    }
    break;
  case FieldDataType::edge:
    for (const auto& edge : primaryFacade->edges())
    {
      indices.push_back(edge.index());
      Point p;
      mesh->get_center(p, edge.index());
      points.push_back(p);
    }
    break;
  case FieldDataType::face:
    for (const auto& face : primaryFacade->faces())
    {
      indices.push_back(face.index());
      Point p;
      mesh->get_center(p, face.index());
      points.push_back(p);
    }
    break;
  case FieldDataType::cell:
    for (const auto& cell : primaryFacade->cells())
    {
      indices.push_back(cell.index());
      Point p;
      mesh->get_center(p, cell.index());
      points.push_back(p);
    }
    break;
  }
}

void ComputeTensorUncertaintyAlgorithmImpl::getPoints(const FieldList& fields)
{
  fieldCount_ = fields.size();

  indices_ = std::vector<std::vector<int>>(fieldCount_);
  points_ = std::vector<std::vector<Point>>(fieldCount_);

  for (size_t f = 0; f < fieldCount_; ++f)
    getPointsForFields(fields[f], indices_[f], points_[f]);
}

void ComputeTensorUncertaintyAlgorithmImpl::verifyData(const FieldList& fields)
{
  fieldSize_ = indices_[0].size();
  for (size_t f = 1; f < fieldCount_; ++f)
    if (indices_[f].size() != fieldSize_)
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE("All field inputs must have the same size.");

  // Verify all are tensors
  for (auto field : fields)
  {
    FieldInformation finfo(field);
    if (!finfo.is_tensor())
      THROW_ALGORITHM_INPUT_ERROR_SIMPLE("This module only supports tensor fields.");
  }
}

void ComputeTensorUncertaintyAlgorithmImpl::getTensors(const FieldList& fields)
{
  tensors_ = std::vector<std::vector<Dyadic3DTensor>>(fieldCount_);
  for (size_t f = 0; f < fieldCount_; ++f)
  {
    tensors_[f] = std::vector<Dyadic3DTensor>(fieldSize_);
    auto vfield = fields[f]->vfield();
    Tensor temp;
    for (size_t v = 0; v < fieldSize_; ++v)
    {
      vfield->get_value(temp, v);
      tensors_[f][v] = scirunTensorToEigenTensor(temp);
      tensors_[f][v].setDescendingRHSOrder();
    }
  }
}

DenseMatrixHandle ComputeTensorUncertaintyAlgorithmImpl::getCovarianceMatrices() const
{
  auto m = std::make_shared<DenseMatrix>(21, fieldSize_);
  for (size_t i = 0; i < fieldSize_; ++i)
  {
    m->col(i) = covarianceMatrices_[i].mandel();
    Eigen::Matrix<double,21,1> recov_m = m->col(i);
    DyadicTensor<6> recov = DyadicTensor<6>(recov_m);
  }
  return m;
}

void ComputeTensorUncertaintyAlgorithmImpl::run(const FieldList& fields)
{
  getPoints(fields);
  verifyData(fields);
  getTensors(fields);
  computeMeanTensors();
  computeCovarianceMatrices();
}

ComputeTensorUncertaintyAlgorithm::ComputeTensorUncertaintyAlgorithm()
{
  addOption(Parameters::MeanInvariantMethod, "Linear Invariant",
            "Linear Invariant|Log-Euclidean|Matrix Average");
  addOption(Parameters::MeanOrientationMethod, "Log-Euclidean",
            "Log-Euclidean|Matrix Average");
}

AlgorithmOutput ComputeTensorUncertaintyAlgorithm::run(const AlgorithmInput& input) const
{
  auto fields = input.getList<Field>(Variables::InputFields);
  auto invariantMethod = get(Parameters::MeanInvariantMethod).toOption();
  auto orientMethod = get(Parameters::MeanOrientationMethod).toOption();
  auto data = runImpl(fields, invariantMethod, orientMethod);

  AlgorithmOutput output;
  output[MeanTensorField] = data.get<0>();
  output[CovarianceMatrix] = data.get<1>();
  return output;
}

boost::tuple<FieldHandle, MatrixHandle> ComputeTensorUncertaintyAlgorithm::runImpl(const FieldList& fields, const AlgoOption& invariantMethod, const AlgoOption& orientationOption) const
{
  if (fields.empty())
    THROW_ALGORITHM_INPUT_ERROR("No input fields given");

  auto impl = ComputeTensorUncertaintyAlgorithmImpl();
  impl.setInvariantMethod(invariantMethod);
  impl.setOrientationMethod(orientationOption);
  impl.run(fields);

  return boost::make_tuple(impl.getMeanTensors(), impl.getCovarianceMatrices());
}
