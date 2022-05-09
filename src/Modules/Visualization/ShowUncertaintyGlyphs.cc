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

#include <Modules/Visualization/ShowUncertaintyGlyphs.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Dyadic3DTensor.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Glyphs/GlyphConstructor.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Graphics/Glyphs/GlyphGeomUtility.h>
#include <Graphics/Glyphs/TensorGlyphBuilder.h>
#include <unsupported/Eigen/MatrixFunctions>

using namespace SCIRun;
using namespace Core;
using namespace Datatypes;
using namespace Modules::Visualization;
using namespace Algorithms;
using namespace Geometry;
using namespace Graphics;
using namespace Graphics::Datatypes;

MODULE_INFO_DEF(ShowUncertaintyGlyphs, Visualization, SCIRun);

namespace SCIRun {
namespace Modules {
namespace Visualization {
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
  GeometryHandle run(const GeometryIDGenerator& idgen, const FieldHandle mean, const MatrixHandle covariance, std::string& idname);
  ShowUncertaintyGlyphsImpl& setShowTensors(bool showTensors);
  ShowUncertaintyGlyphsImpl& setTensorsResolution(int resolution);
  ShowUncertaintyGlyphsImpl& setTensorsTransparency(bool useTransparency);
  ShowUncertaintyGlyphsImpl& setTensorsUniformTransparencyValue(double transparency);
  ShowUncertaintyGlyphsImpl& setTensorsSuperquadricEmphasis(double emphasis);
  ShowUncertaintyGlyphsImpl& setTensorsScale(double scale);

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

  bool showTensors_ = true;
  double emphasis_ = 3.0;
  double scale_ = 1.0;
  bool useTensorTransparency_ = true;
  double transparency_ = 0.5;
  int resolution_ = 101;
};

ShowUncertaintyGlyphsImpl::ShowUncertaintyGlyphsImpl()
{
  constructor_ = GlyphConstructor();
}

ShowUncertaintyGlyphsImpl& ShowUncertaintyGlyphsImpl::setShowTensors(bool showTensors)
{
  showTensors_ = showTensors;
  return *this;
}

ShowUncertaintyGlyphsImpl& ShowUncertaintyGlyphsImpl::setTensorsResolution(int resolution)
{
  resolution_ = resolution;
  return *this;
}

ShowUncertaintyGlyphsImpl& ShowUncertaintyGlyphsImpl::setTensorsTransparency(bool useTransparency)
{
  useTensorTransparency_ = useTransparency;
  return *this;
}

ShowUncertaintyGlyphsImpl& ShowUncertaintyGlyphsImpl::setTensorsUniformTransparencyValue(double transparency)
{
  transparency_ = transparency;
  return *this;
}

ShowUncertaintyGlyphsImpl& ShowUncertaintyGlyphsImpl::setTensorsSuperquadricEmphasis(double emphasis)
{
  emphasis_ = emphasis;
  return *this;
}

ShowUncertaintyGlyphsImpl& ShowUncertaintyGlyphsImpl::setTensorsScale(double scale)
{
  scale_ = scale;
  return *this;
}

GeometryHandle ShowUncertaintyGlyphsImpl::run(
  const GeometryIDGenerator& idgen, const FieldHandle mean, const MatrixHandle covariance, std::string& idname)
{
  if (!showTensors_) return std::make_shared<GeometryObjectSpire>(idgen, idname, true);
  getPoints(mean);
  fieldSize_ = points_.size();
  computeOffsetSurface(mean, covariance);


  RenderState renState;
  renState.set(RenderState::ActionFlags::USE_NORMALS, true);
  renState.set(RenderState::ActionFlags::IS_ON, true);
  renState.set(RenderState::ActionFlags::USE_TRANSPARENCY, true);
  renState.mGlyphType = RenderState::GlyphType::SUPERQUADRIC_TENSOR_GLYPH;
  renState.defaultColor = ColorRGB(1.0, 1.0, 1.0);
  renState.set(RenderState::ActionFlags::USE_DEFAULT_COLOR, true);
  auto vmesh = mean->vmesh();

  auto geom(std::make_shared<GeometryObjectSpire>(idgen, idname, true));
  constructor_.buildObject(*geom, geom->uniqueID(), true, transparency_, ColorScheme::COLOR_UNIFORM, renState,
      vmesh->get_bounding_box(), true, nullptr);

  return geom;
}

void ShowUncertaintyGlyphsImpl::computeOffsetSurface(FieldHandle mean, MatrixHandle covariance)
{
  auto vfield = mean->vfield();
  auto denseCovariance = *castMatrix::toDense(covariance);
  Tensor t;
  for (int i = 0; i < fieldSize_; ++i)
  {
    vfield->get_value(t, i);
    Dyadic3DTensor eigT = scirunTensorToEigenTensor(t);
    eigT.setDescendingRHSOrder();
    Eigen::Matrix<double, 21, 1> mandel = denseCovariance.col(i);
    DyadicTensor<6> covT = DyadicTensor<6>(mandel);

    UncertaintyTensorOffsetSurfaceBuilder builder(eigT, points_[i], emphasis_, covT.asMatrix());
    builder.setResolution(resolution_);
    builder.scaleTensor(scale_);
    builder.scaleCovariance(scale_);
    builder.generateOffsetSurface(constructor_);
  }
}

void ShowUncertaintyGlyphsImpl::getPoints(const FieldHandle& field)
{
  getPointsForField(field, indices_, points_);
}

void ShowUncertaintyGlyphsImpl::getPointsForField(
    FieldHandle field, std::vector<int>& indices, std::vector<Point>& points)
{
  // Collect indices and points from facades
  int fieldLocation = 0;
  FieldInformation finfo(field);
  if (finfo.is_point() || finfo.is_linear())
    fieldLocation = Mesh::NODES_E;
  else if (finfo.is_line())
    fieldLocation = Mesh::EDGES_E;
  else if (finfo.is_surface())
    fieldLocation = Mesh::FACES_E;
  else
    fieldLocation = Mesh::CELLS_E;

  auto mesh = field->vmesh();
  auto primaryFacade = field->mesh()->getFacade();
  switch (fieldLocation)
  {
  case Mesh::NODES_E:
    for (const auto& node : primaryFacade->nodes())
    {
      indices.push_back(node.index());
      Point p;
      mesh->get_center(p, node.index());
      points.push_back(p);
    }
    break;
  case Mesh::EDGES_E:
    for (const auto& edge : primaryFacade->edges())
    {
      indices.push_back(edge.index());
      Point p;
      mesh->get_center(p, edge.index());
      points.push_back(p);
    }
    break;
  case Mesh::FACES_E:
    for (const auto& face : primaryFacade->faces())
    {
      indices.push_back(face.index());
      Point p;
      mesh->get_center(p, face.index());
      points.push_back(p);
    }
    break;
  case Mesh::CELLS_E:
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


ShowUncertaintyGlyphs::ShowUncertaintyGlyphs() : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputGeom);
}

void ShowUncertaintyGlyphs::setStateDefaults()
{
  auto state = get_state();
  state->setValue(FieldName, std::string());
  state->setValue(ShowTensors, true);
  state->setValue(TensorsResolution, 101);
  state->setValue(TensorsTransparency, true);
  state->setValue(TensorsUniformTransparencyValue, 0.5);
  state->setValue(SuperquadricEmphasis, 3.0);
  state->setValue(TensorsScale, 1.0);
}

void ShowUncertaintyGlyphs::execute()
{
  auto mean = getRequiredInput(InputField);
  auto covariance = getRequiredInput(InputMatrix);

  auto state = get_state();
  if(needToExecute())
  {
    auto impl = ShowUncertaintyGlyphsImpl();
    impl.setShowTensors(state->getValue(ShowTensors).toBool())
      .setTensorsResolution(state->getValue(TensorsResolution).toInt())
      .setTensorsTransparency(state->getValue(TensorsTransparency).toBool())
      .setTensorsUniformTransparencyValue(state->getValue(TensorsUniformTransparencyValue).toDouble())
      .setTensorsSuperquadricEmphasis(state->getValue(SuperquadricEmphasis).toDouble())
      .setTensorsScale(state->getValue(TensorsScale).toDouble());

    // Creates id
    std::string idname = "ShowUncertaintyGlyphs";
    if(!state->getValue(FieldName).toString().empty())
      idname += GeometryObject::delimiter + state->getValue(FieldName).toString() +
        " (from " + id().id_ +")";
    sendOutput(OutputGeom, impl.run(*this, mean, covariance, idname));
  }
}

}}}


const AlgorithmParameterName ShowUncertaintyGlyphs::FieldName("FieldName");
const AlgorithmParameterName ShowUncertaintyGlyphs::ShowTensors("ShowTensors");
const AlgorithmParameterName ShowUncertaintyGlyphs::TensorsTransparency("TensorsTransparency");
const AlgorithmParameterName ShowUncertaintyGlyphs::TensorsUniformTransparencyValue("TensorsUniformTransparencyValue");
const AlgorithmParameterName ShowUncertaintyGlyphs::SuperquadricEmphasis("SuperquadricEmphasis");
const AlgorithmParameterName ShowUncertaintyGlyphs::TensorsScale("TensorsScale");
const AlgorithmParameterName ShowUncertaintyGlyphs::TensorsResolution("TensorsResolution");
