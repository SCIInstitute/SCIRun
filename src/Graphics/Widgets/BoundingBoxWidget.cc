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

#include <Core/Datatypes/Feedback.h>
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/Dyadic3DTensor.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Graphics/Widgets/BoundingBoxWidget.h>
#include <Graphics/Widgets/GlyphFactory.h>
#include <Graphics/Widgets/WidgetBuilders.h>
#include <Graphics/Widgets/WidgetFactory.h>
#include <glm/gtx/matrix_operation.hpp>

using namespace SCIRun;
using namespace Graphics::Datatypes;
using namespace Core::Geometry;
using namespace Core::Datatypes;

// Index diagram for corners and edges:
// NOTE: This ordering is based on the order corners are generated.
// If you change order of corners, update this diagram.
//
//          (0)          1            (1)
//           .+-----------------------+
//         .' |                     .'|
//       8.'  |      [4]         9.'  |
//      .'    |     5           .'    |
//  (4)+------+----------------+(5)   |
//     |      |                |      |
//     |      |         [0]    |      |
//     |    0 |                |      | 2
//     |      |                |      |
//    4| [3]  |               6| [1]  |
//     |      |  [2]           |      |
//     |      |                |      |
//     |   (3)+----------------+------+(2)
//     |    .'          3      |    .'
//     |  .'11      [5]        |  .' 10
//     |.'                     |.'
//     +-----------------------+
//   (7)            7          (6)
//
//  Legend: (#) - corner index
//          [#] - face index
//           #  - edge index

BBoxDataHandler::BBoxDataHandler(const Point& center, const std::vector<Vector>& scaledEigvecs) :
  TRANSLATE_COLOR_(ColorRGB(1.0, 1.0, 1.0).toString()),
  ROTATE_COLOR_(ColorRGB(1.0, 0.379, 0.531).toString()),
  SCALE_COLOR_(ColorRGB(0.66, 0.859, 0.461).toString()),
  SCALE_AXIS_COLOR_(ColorRGB(0.469, 0.859, 0.906).toString())
{
  scaledEigvecs_ = scaledEigvecs;
  scale_ = (scaledEigvecs_[0] + scaledEigvecs_[1] + scaledEigvecs_[2]).length();

  corners_.resize(CORNER_COUNT_);
  edges_.resize(EDGE_COUNT_);
  faceDisks_.resize(FACE_COUNT_);
  faceSpheres_.resize(FACE_COUNT_);
  facePoints_.resize(FACE_COUNT_);

  for (auto f = 0; f < FACE_COUNT_; ++f)
    facePoints_[f] = center + getDirectionOfFace(f);

  cornerPoints_ = {
    center + getDirectionOfFace(0) + getDirectionOfFace(4) + getDirectionOfFace(3),
    center + getDirectionOfFace(0) + getDirectionOfFace(4) + getDirectionOfFace(1),
    center + getDirectionOfFace(0) + getDirectionOfFace(5) + getDirectionOfFace(1),
    center + getDirectionOfFace(0) + getDirectionOfFace(5) + getDirectionOfFace(3),
    center + getDirectionOfFace(2) + getDirectionOfFace(4) + getDirectionOfFace(3),
    center + getDirectionOfFace(2) + getDirectionOfFace(4) + getDirectionOfFace(1),
    center + getDirectionOfFace(2) + getDirectionOfFace(5) + getDirectionOfFace(1),
    center + getDirectionOfFace(2) + getDirectionOfFace(5) + getDirectionOfFace(3)};
}

size_t BBoxDataHandler::getIndexOfDirectionOfFace(int f) const
{
  switch (f)
  {
  case 0:
  case 2:
    return 0;
  case 4:
  case 5:
    return 1;
  case 1:
  case 3:
    return 2;
  default:
    THROW_INVALID_ARGUMENT("The argument is not in a valid range of face indices.");
  }
}

Vector BBoxDataHandler::getDirectionOfFace(int f) const
{
  switch (f)
  {
  case 0:
    return scaledEigvecs_[0];
  case 1:
    return -scaledEigvecs_[2];
  case 2:
    return -scaledEigvecs_[0];
  case 3:
    return scaledEigvecs_[2];
  case 4:
    return scaledEigvecs_[1];
  case 5:
    return -scaledEigvecs_[1];
  default:
    THROW_INVALID_ARGUMENT("The argument is not in a valid range of face indices.");
  }
}

glm::mat4 BBoxDataHandler::getScaleTrans() const
{
  auto scaleVec = glm::vec4(static_cast<float>(scaledEigvecs_[0].length()),
          static_cast<float>(scaledEigvecs_[1].length()),
          static_cast<float>(scaledEigvecs_[2].length()), 1.0f);

  return glm::diagonal4x4(scaleVec);
}

std::vector<WidgetHandle> BBoxDataHandler::getEdges() const
{
  return edges_;
}

std::vector<WidgetHandle> BBoxDataHandler::getCorners() const
{
  return corners_;
}

std::vector<WidgetHandle> BBoxDataHandler::getFaceSpheres() const
{
  return faceSpheres_;
}

std::vector<WidgetHandle> BBoxDataHandler::getFaceDisks() const
{
  return faceDisks_;
}

std::vector<WidgetHandle> BBoxDataHandler::getCornersOfFace(int f)
{
  assert(f >= 0);
  assert(f < FACE_COUNT_);
  switch (f)
  {
  case 0:
    return {corners_[0], corners_[1], corners_[2], corners_[3]};
  case 1:
    return {corners_[1], corners_[2], corners_[5], corners_[6]};
  case 2:
    return {corners_[4], corners_[5], corners_[6], corners_[7]};
  case 3:
    return {corners_[0], corners_[3], corners_[4], corners_[7]};
  case 4:
    return {corners_[0], corners_[1], corners_[4], corners_[5]};
  case 5:
    return {corners_[2], corners_[3], corners_[6], corners_[7]};
  default:
    THROW_INVALID_ARGUMENT("The argument is not in a valid range of face indices.");
  }
}

std::vector<WidgetHandle> BBoxDataHandler::getEdgesOfFace(int f)
{
  assert(f >= 0);
  assert(f < FACE_COUNT_);
  switch (f)
  {
  case 0:
    return {edges_[0], edges_[1], edges_[2], edges_[3]};
  case 1:
    return {edges_[2], edges_[6], edges_[9], edges_[10]};
  case 2:
    return {edges_[4], edges_[5], edges_[6], edges_[7]};
  case 3:
    return {edges_[0], edges_[4], edges_[8], edges_[11]};
  case 4:
    return {edges_[1], edges_[5], edges_[8], edges_[9]};
  case 5:
    return {edges_[3], edges_[7], edges_[10], edges_[11]};
  default:
    THROW_INVALID_ARGUMENT("The argument is not in a valid range of face indices.");
  }
}

int BBoxDataHandler::getOppositeFaceIndex(int f)
{
  assert(f >= 0);
  assert(f < FACE_COUNT_);
  switch (f)
  {
  case 0:
    return 2;
  case 1:
    return 3;
  case 2:
    return 0;
  case 3:
    return 1;
  case 4:
    return 5;
  case 5:
    return 4;
  default:
    THROW_INVALID_ARGUMENT("The argument is not in a valid range of face indices.");
  }
}

std::vector<WidgetHandle> BBoxDataHandler::getWidgetsOnFace(int f)
{
  assert(f >= 0);
  assert(f < FACE_COUNT_);
  auto wids = getCornersOfFace(f);
  auto edgeWids = getEdgesOfFace(f);
  wids.insert(wids.end(), edgeWids.begin(), edgeWids.end());
  wids.push_back(faceDisks_[f]);
  wids.push_back(faceSpheres_[f]);
  return wids;
}

std::vector<WidgetHandle> BBoxDataHandler::getWidgetsOnOppositeFace(int f)
{
  assert(f >= 0);
  assert(f < FACE_COUNT_);
  auto oppF = getOppositeFaceIndex(f);
  auto wids = getCornersOfFace(oppF);
  auto edgeWids = getEdgesOfFace(oppF);
  wids.insert(wids.end(), edgeWids.begin(), edgeWids.end());
  wids.push_back(faceDisks_[oppF]);
  wids.push_back(faceSpheres_[oppF]);
  return wids;
}

std::vector<WidgetHandle> BBoxDataHandler::getFaceWidgetsParrallelToFace(int f)
{
  assert(f >= 0);
  assert(f < FACE_COUNT_);
  switch (f)
  {
  case 0:
  case 2:
    return { faceDisks_[1], faceDisks_[3], faceDisks_[4], faceDisks_[5],
      faceSpheres_[1], faceSpheres_[3], faceSpheres_[4], faceSpheres_[5]};
  case 1:
  case 3:
    return { faceDisks_[0], faceDisks_[2], faceDisks_[4], faceDisks_[5],
      faceSpheres_[0], faceSpheres_[2], faceSpheres_[4], faceSpheres_[5]};
  case 4:
  case 5:
    return { faceDisks_[0], faceDisks_[1], faceDisks_[2], faceDisks_[3],
      faceSpheres_[0], faceSpheres_[1], faceSpheres_[2], faceSpheres_[3]};
  default:
    THROW_INVALID_ARGUMENT("The argument is not in a valid range of face indices.");
  }
}

std::vector<WidgetHandle> BBoxDataHandler::getEdgesParrallelToFace(int f)
{
  switch (f)
  {
  case 0:
  case 2:
    return {edges_[8], edges_[9], edges_[10], edges_[11]};
  case 1:
  case 3:
    return {edges_[1], edges_[3], edges_[5], edges_[7]};
  case 4:
  case 5:
    return {edges_[0], edges_[2], edges_[4], edges_[6]};
  default:
    THROW_INVALID_ARGUMENT("The argument is not in a valid range of face indices.");
  }
}

void BBoxDataHandler::makeCylinders(const GeneralWidgetParameters& gen,
                                    const CommonWidgetParameters& params,
                                    WidgetBase& widget)
{
  const static double cylinderRadius = 1;

  auto builder = CylinderWidgetBuilder(gen.base.idGenerator)
    .transformMapping({{WidgetInteraction::CLICK, singleMovementWidget(WidgetMovement::TRANSLATE)}})
                       //WidgetInteraction::RIGHT_CLICK, singleMovementWidget(WidgetMovement::TRANSLATE_AXIS)}})
    .scale(cylinderRadius * params.scale * scale_)
    .defaultColor(TRANSLATE_COLOR_)
    .boundingBox(params.bbox)
    .resolution(params.resolution);

  edges_ = {
    builder.tag("Edge0").diameterPoints(cornerPoints_[0], cornerPoints_[3]).build(),
    builder.tag("Edge1").diameterPoints(cornerPoints_[0], cornerPoints_[1]).build(),
    builder.tag("Edge2").diameterPoints(cornerPoints_[1], cornerPoints_[2]).build(),
    builder.tag("Edge3").diameterPoints(cornerPoints_[2], cornerPoints_[3]).build(),
    builder.tag("Edge4").diameterPoints(cornerPoints_[4], cornerPoints_[7]).build(),
    builder.tag("Edge5").diameterPoints(cornerPoints_[4], cornerPoints_[5]).build(),
    builder.tag("Edge6").diameterPoints(cornerPoints_[5], cornerPoints_[6]).build(),
    builder.tag("Edge7").diameterPoints(cornerPoints_[6], cornerPoints_[7]).build(),
    builder.tag("Edge8").diameterPoints(cornerPoints_[0], cornerPoints_[4]).build(),
    builder.tag("Edge9").diameterPoints(cornerPoints_[1], cornerPoints_[5]).build(),
    builder.tag("Edge10").diameterPoints(cornerPoints_[2], cornerPoints_[6]).build(),
    builder.tag("Edge11").diameterPoints(cornerPoints_[3], cornerPoints_[7]).build()};

  for (auto edge : getEdgesParrallelToFace(0))
    edge->addTransformParameters<AxisTranslation>(getDirectionOfFace(0));
  for (auto edge : getEdgesParrallelToFace(3))
    edge->addTransformParameters<AxisTranslation>(getDirectionOfFace(3));
  for (auto edge : getEdgesParrallelToFace(4))
    edge->addTransformParameters<AxisTranslation>(getDirectionOfFace(4));
}

void BBoxDataHandler::makeCornerSuperquadrics(const GeneralWidgetParameters& gen,
                                              const CommonWidgetParameters& params,
                                              WidgetBase& widget)
{
  const static double cornerSphereRadius = 1.5;
  const static double cornerEmphasis = 0.4;

  auto builder = SuperquadricWidgetBuilder(gen.base.idGenerator)
    .transformMapping({{WidgetInteraction::CLICK, singleMovementWidget(WidgetMovement::SCALE)}})
    .scale(cornerSphereRadius * params.scale * scale_)
    .origin(params.origin)
    .defaultColor(SCALE_COLOR_)
    .boundingBox(params.bbox)
    .resolution(params.resolution);

  auto t = Tensor(scaledEigvecs_[0].normal(), scaledEigvecs_[1].normal(), scaledEigvecs_[2].normal());
  auto newT = Dyadic3DTensor(t.xx(), t.xy(), t.xz(), t.yy(), t.yz(), t.zz());
  for (int c = 0; c < CORNER_COUNT_; ++c)
  {
    corners_[c] = builder.tag("Corner" + std::to_string(c)).centerPoint(cornerPoints_[c])
      .tensor(newT).A(cornerEmphasis).B(cornerEmphasis).build();
    auto flipVec = cornerPoints_[c] - params.origin;
    corners_[c]->addTransformParameters<Scaling>(params.origin, flipVec);
  }
}

void BBoxDataHandler::makeFaceSpheres(const GeneralWidgetParameters& gen,
                                      const CommonWidgetParameters& params,
                                      WidgetBase& widget)
{
  const static double faceSphereRadius = 1.5;

  auto builder = SphereWidgetBuilder(gen.base.idGenerator)
    .transformMapping({{WidgetInteraction::CLICK, singleMovementWidget(WidgetMovement::ROTATE)}})
    .scale(faceSphereRadius * params.scale * scale_)
    .origin(params.origin)
    .defaultColor(ROTATE_COLOR_)
    .boundingBox(params.bbox)
    .resolution(params.resolution);

  for (int f = 0; f < FACE_COUNT_; ++f)
  {
    faceSpheres_[f] = builder.tag("FaceSphere" + std::to_string(f)).centerPoint(facePoints_[f]).build();
    faceSpheres_[f]->addTransformParameters<Rotation>(params.origin);
  }
}

void BBoxDataHandler::makeFaceDisks(const GeneralWidgetParameters& gen,
                                    const CommonWidgetParameters& params,
                                    WidgetBase& widget)
{
  const static double diskDiameterScale = 1.0;
  const static double diskLengthScale = 2.5;

  auto builder = DiskWidgetBuilder(gen.base.idGenerator)
    .transformMapping({{WidgetInteraction::CLICK, singleMovementWidget(WidgetMovement::NONE)}})
    // .transformMapping({{WidgetInteraction::CLICK, singleMovementWidget(WidgetMovement::SCALE_AXIS)}})
    //.transformMapping({{WidgetInteraction::CLICK, singleMovementWidget(WidgetMovement::TRANSLATE_AXIS)}})
    .scale(diskDiameterScale * params.scale * scale_)
    .defaultColor(SCALE_AXIS_COLOR_)
    .boundingBox(params.bbox)
    .resolution(params.resolution);

  for (int f = 0; f < FACE_COUNT_; ++f)
  {
    auto axis = getDirectionOfFace(f).normal();
    // auto scaleAxisIndex = getIndexOfDirectionOfFace(f);
    faceDisks_[f] = builder.tag("FaceDisk" + std::to_string(f))
      .diameterPoints(facePoints_[f], facePoints_[f] + axis * scale_ * params.scale * diskLengthScale)
      .build();
    // faceDisks_[f]->addTransformParameters<AxisScaling>(params.origin, axis, scaleAxisIndex);
    //faceDisks_[f]->addTransformParameters<AxisTranslation>(axis);
  }
}

BoundingBoxWidget::BoundingBoxWidget(const GeneralWidgetParameters& gen,
                                     BoundingBoxParameters params) : CompositeWidget(gen.base)
{
  BBoxDataHandler boxData(params.pos.center_, params.pos.scaledEigvecs_);
  boxData.makeCylinders(gen, params.common, *this);
  boxData.makeCornerSuperquadrics(gen, params.common, *this);
  boxData.makeFaceSpheres(gen, params.common, *this);
  boxData.makeFaceDisks(gen, params.common, *this);

  auto edges = boxData.getEdges();
  auto corners = boxData.getCorners();
  auto faceSpheres = boxData.getFaceSpheres();
  auto faceDisks = boxData.getFaceDisks();

  widgets_ = edges;
  widgets_.insert(widgets_.end(), corners.begin(), corners.end());
  widgets_.insert(widgets_.end(), faceSpheres.begin(), faceSpheres.end());
  widgets_.insert(widgets_.end(), faceDisks.begin(), faceDisks.end());

  for (auto& edge : edges)
  {
    edge << propagatesEvent<WidgetMovement::TRANSLATE>::to << TheseWidgets{widgets_};
    // edge << propagatesEvent<WidgetMovement::TRANSLATE_AXIS>::to << TheseWidgets{widgets_};
  }
  for (auto& corner : corners)
    corner << propagatesEvent<WidgetMovement::SCALE>::to << TheseWidgets{widgets_};
  for (auto& faceSphere : faceSpheres)
    faceSphere << propagatesEvent<WidgetMovement::ROTATE>::to << TheseWidgets{widgets_};
  // for (auto i = 0; i < boxData.FACE_COUNT_; ++i)
  // {
    // auto edgesParallel = boxData.getEdgesParrallelToFace(i);
    // auto face = boxData.getWidgetsOnFace(i);
    // auto oppositeFace = boxData.getWidgetsOnOppositeFace(i);

    //faceDisks[i] << propagatesEvent<WidgetMovement::SCALE_AXIS>::to << TheseWidgets{edgesParallel};
    //faceDisks[i] << propagatesEvent<WidgetMovement::TRANSLATE_AXIS>::to << TheseWidgets{face};
  // }
}


  // face 0 = {v0,v1,v2,v3}
  // axis translate
  // fd0 translates along an axis--T0
  // all v from face 0, {v0,v1,v2,v3}, also translate--T0
  // all e from face 0, {e0,e1,e2,e3} also translate--T0
  // opposite face {v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, is fixed--Id
  // parallel edges--{e8,e9,e10,e11}--uni-scale/translate along axis--T1
#if 0
  anything else on parallel faces {fs/fd...} --T0/2
  void wireTheAboveMovementScheme(clicked, translated, fixed?, uniscaled+translated, halfTranslated)
  {
    clicked << propagatesEvent<WidgetMovement::TRANSLATE>::to << TheseWidgets {translated};
    clicked << propagatesEvent<WidgetMovement::TRANSLATE_AXIS>::to << uniscaled+translated;
    clicked << propagatesEvent<WidgetMovement::TRANSLATE_AXIS_HALF>::to << halfTranslated;
  }

  for (int f = 0; f < 6; ++6)
  {
    wireTheAboveMovementScheme(f, )
  }

  wireTheAboveMovementScheme(fd0, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  //can we use mesh geometry functions to get all the relative components functionally?
  wireTheAboveMovementScheme(fd1, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  wireTheAboveMovementScheme(clicked, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  wireTheAboveMovementScheme(clicked, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  wireTheAboveMovementScheme(clicked, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  wireTheAboveMovementScheme(clicked, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  #endif

// Pseudocode
#if 0
BoundingBoxWidget::BoundingBoxWidget(const GeneralWidgetParameters& gen,
  BoundingBoxParameters params) : CompositeWidget(gen.base)
{
  // vertices--sphere widgets
  // left-click: scale
  // right-click: axis scale
  WidgetHandle v0,v1,v2,v3,v4,v5,v6,v7;

  // edges--cylinders
  // left-click: translate
  // right-click: axis translate
  WidgetHandle e0,e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11;
  // build step:
  ...
  .transformMapping({
    {WidgetInteraction::CLICK, WidgetMovement::TRANSLATE},
    {WidgetInteraction::RIGHT_CLICK, WidgetMovement::AXIS_TRANSLATE}
  })
  ...
  auto edges = std::vector<WidgetHandle>{e0,e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11};

  // faces part 1--disks
  // left-click: translate/bidirectional scale
  // right-click: translate/unidirectional scale
  WidgetHandle fd0, fd1, fd2, fd3, fd4, fd5;

  //TODO:
  // face spheres
  WidgetHandle fs0, fs1, fs2, fs3, fs4, fs5;

  for (auto& e : edges)
  {
    registerAllSiblingWidgetsForEvent(e, WidgetMovement::TRANSLATE);
  }

  // face 0 = {v0,v1,v2,v3}
  // axis translate
  fd0 translates along an axis--T0
  all v from face 0, {v0,v1,v2,v3}, also translate--T0
  all e from face 0, {e0,e1,e2,e3} also translate--T0
  opposite face {v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, is fixed--Id
  parallel edges--{e8,e9,e10,e11}--uni-scale/translate along axis--T1
  anything else on parallel faces {fs/fd...} --T0/2

  void wireTheAboveMovementScheme(clicked, translated, fixed?, uniscaled+translated, halfTranslated)
  {
    clicked << propagatesEvent<WidgetMovement::TRANSLATE>::to << TheseWidgets {translated};
    clicked << propagatesEvent<WidgetMovement::UNISCALED_TRANSLATED>::to << uniscaled+translated;
    clicked << propagatesEvent<WidgetMovement::MODIFIED_TRANSLATE>::to << halfTranslated;
  }

  wireTheAboveMovementScheme(fd0, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  //can we use mesh geometry functions to get all the relative components functionally?

  wireTheAboveMovementScheme(fd1, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  wireTheAboveMovementScheme(clicked, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  wireTheAboveMovementScheme(clicked, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  wireTheAboveMovementScheme(clicked, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
  wireTheAboveMovementScheme(clicked, translated={v0,v1,v2,v3,e0,e1,e2,e3},
    ? id/left alone={v4,v5,v6,v7},{e4,e5,e6,e7},{fd5,fs5}, //depends on testability
    uniscaledParallelEdges = {e8,e9,e10,e11}
  );
}
#endif
