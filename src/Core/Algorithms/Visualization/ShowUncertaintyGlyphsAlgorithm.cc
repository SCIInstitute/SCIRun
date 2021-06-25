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

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Visualization/ShowUncertaintyGlyphsAlgorithm.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Dyadic3DTensor.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Logging/Log.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Glyphs/GlyphConstructor.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Graphics/Glyphs/GlyphGeomUtility.h>
#include <Graphics/Glyphs/TensorGlyphBuilder.h>
#include <unsupported/Eigen/MatrixFunctions>

using namespace SCIRun;
using namespace Graphics;
using namespace Graphics::Datatypes;
using namespace Core;
using namespace Core::Datatypes;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Core::Algorithms::Visualization;

using GGU = GlyphGeomUtility;

const AlgorithmOutputName ShowUncertaintyGlyphsAlgorithm::OutputGeom("OutputGeom");

Dyadic3DTensor scirunTensorToEigenTensor(const Geometry::Tensor& t)
{
  Dyadic3DTensor newTensor(t.xx(), t.xy(), t.xz(), t.yy(), t.yz(), t.zz());
  return newTensor;
}

Tensor eigenTensorToScirunTensor(const Dyadic3DTensor& t)
{
  return Tensor(t(0, 0), t(1, 0), t(2, 0), t(1, 1), t(2, 1), t(2, 2));
}

class ShowUncertaintyGlyphsImpl
{
 public:
  ShowUncertaintyGlyphsImpl();
  GeometryHandle run(const GeometryIDGenerator& idgen, const FieldHandle mean, const MatrixHandle covariance);

 private:
  void verifyData(const FieldList& fields);
  void getPoints(const FieldHandle& fields);
  void getPointsForField(FieldHandle field, std::vector<int>& indices, std::vector<Point>& points);
  void computeOffsetSurface(FieldHandle mean, MatrixHandle covariance);

  GlyphConstructor constructor_;
  int fieldCount_ = 0;
  int fieldSize_ = 0;

  std::vector<Point> points_;
  std::vector<int> indices_;
  std::vector<Dyadic3DTensor> meanTensors_;
  std::vector<Eigen::Matrix<double, 6, 6>> covarianceMatrices_;

  // TODO make ui params
  double emphasis_ = 3.0;
  int resolution_ = 100;
};

ShowUncertaintyGlyphsAlgorithm::ShowUncertaintyGlyphsAlgorithm() {}

ShowUncertaintyGlyphsImpl::ShowUncertaintyGlyphsImpl()
{
  constructor_ = GlyphConstructor();
}

// TODO move to separate algo
enum FieldDataType
{
  node,
  edge,
  face,
  cell
};

GeometryHandle ShowUncertaintyGlyphsImpl::run(
  const GeometryIDGenerator& idgen, const FieldHandle mean, const MatrixHandle covariance)
{
  getPoints(mean);
  // verifyData(mean);
  // getTensors(mean);
  // computeMeanTensors();
  // computeCovarianceMatrices();
  fieldSize_ = points_.size();
  // std::cout << "computing offset...\n";
  computeOffsetSurface(mean, covariance);
  // std::cout << "done computing offset\n";

  // Creates id
  std::string idname = "ShowUncertaintyGlyphs";
  // if(!state->getValue(ShowFieldGlyphs::FieldName).toString().empty())
  // idname += GeometryObject::delimiter + state->getValue(ShowFieldGlyphs::FieldName).toString() +
  // " (from " + moduleId_ +")";

  RenderState renState;
  renState.set(RenderState::USE_NORMALS, true);
  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENCY, true);
  renState.mGlyphType = RenderState::GlyphType::SUPERQUADRIC_TENSOR_GLYPH;
  renState.defaultColor = ColorRGB(1.0, 1.0, 1.0);
  renState.set(RenderState::USE_DEFAULT_COLOR, true);
  SpireIBO::PRIMITIVE primIn = SpireIBO::PRIMITIVE::TRIANGLES;
  auto vmesh = mean->vmesh();
  // std::cout << "bbox: " << vmesh->get_bounding_box() << "\n";

  auto geom(boost::make_shared<GeometryObjectSpire>(idgen, idname, true));
  constructor_.buildObject(*geom, geom->uniqueID(), true, 0.5, ColorScheme::COLOR_UNIFORM, renState,
      primIn, vmesh->get_bounding_box(), true, nullptr);

  return geom;
}

void ShowUncertaintyGlyphsImpl::computeOffsetSurface(FieldHandle mean, MatrixHandle covariance)
{
  auto vfield = mean->vfield();
  auto denseCovariance = *castMatrix::toDense(covariance);
  Tensor t;
  for (int i = 0; i < fieldSize_; ++i)
  // for (int i = 36; i < 37; ++i)
  {
    vfield->get_value(t, i);
    Dyadic3DTensor eigT = scirunTensorToEigenTensor(t);
    eigT.setDescendingRHSOrder();
    Eigen::Matrix<double, 21, 1> mandel = denseCovariance.col(i);
    DyadicTensor<6> covT = DyadicTensor<6>(mandel);

    std::cout << "mean:\n" << eigT << "\n";
    std::cout << "covariance:\n" << covT << "\n";
    std::cout << "covariance mandel:\n" << mandel << "\n";
    // std::cout << "cov in SUG:\n" << covT.asMatrix() << "\n";
    UncertaintyTensorOffsetSurfaceBuilder builder(eigT, points_[i], emphasis_);
    builder.setResolution(51);
    builder.generateOffsetSurface(constructor_, covT.asMatrix());
  }
}

void ShowUncertaintyGlyphsImpl::getPoints(const FieldHandle& field)
{
  // indices_ = std::vector<int>(fieldSize_);
  // points_ = std::vector<Point>(fieldSize_);

  getPointsForField(field, indices_, points_);
}

void ShowUncertaintyGlyphsImpl::getPointsForField(
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


// main algorithm function
AlgorithmOutput ShowUncertaintyGlyphsAlgorithm::run(
    const GeometryIDGenerator& idGen, const AlgorithmInput& input) const
{
  auto mean = input.get<Field>(Variables::InputField);
  auto covariance = input.get<Matrix>(Variables::InputMatrix);

  auto impl = ShowUncertaintyGlyphsImpl();

  AlgorithmOutput output;
  output[OutputGeom] = impl.run(idGen, mean, covariance);
  return output;
}

AlgorithmOutput ShowUncertaintyGlyphsAlgorithm::run(const AlgorithmInput& input) const
{
  auto mean = input.get<Field>(Variables::InputField);
  auto covariance = input.get<Matrix>(Variables::InputMatrix);
  auto impl = ShowUncertaintyGlyphsImpl();

  AlgorithmOutput output;
  return output;
}
