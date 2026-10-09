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

#include "VtkRenderer.h"
#include <Core/Algorithms/Visualization/VtkIncludes.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <iostream>
#include "VtkOrientationOverlay.h"
#include "VtkScaleBarOverlay.h"
#include "VtkOverlay.h"

using namespace SCIRun;
using namespace Render;
using namespace Core::Datatypes;
using namespace Core::Geometry;

#ifdef WITH_VTK

VtkRenderer::VtkRenderer() {}

VtkRenderer::~VtkRenderer() {}

// Rendering-----------------------------------------------------------------------------------------
void VtkRenderer::renderFrame()
{
  if (!initialized_) return;

  renderWindow_->SetSize(width_, height_);

  renderer_->ResetCameraClippingRange();

  renderWindow_->Render();

  w2i_->Modified();
  w2i_->Update();

  vtkImageData* image = w2i_->GetOutput();

  int dims[3];
  image->GetDimensions(dims);

  int numComponents = image->GetNumberOfScalarComponents();

  imagePixels_ = static_cast<unsigned char*>(image->GetScalarPointer());

  if (numComponents == 3)
  {
    int bytesPerLine = dims[0] * 3;

    image_ = QImage(imagePixels_, dims[0], dims[1], bytesPerLine, QImage::Format_RGB888).copy();
  }
  else if (numComponents == 4)
  {
    int bytesPerLine = dims[0] * 4;

    image_ = QImage(imagePixels_, dims[0], dims[1], bytesPerLine, QImage::Format_RGBA8888).copy();
  }

// VTK is usually vertically flipped relative to Qt
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
  image_ = image_.flipped(Qt::Vertical);
#else
  image_ = image_.mirrored(false, true);
#endif

  // std::cout << "Rendered image size: " << image_.width() << "x" << image_.height() << std::endl;
}

// Interaction---------------------------------------------------------------------------------------
void VtkRenderer::resize(uint32_t width, uint32_t height, double dpr)
{
  width_ = width;
  height_ = height;

  if (dpr != devicePixelRatio_)
  {
    devicePixelRatio_ = dpr;
    image_.setDevicePixelRatio(devicePixelRatio_);
  }

  if (renderWindow_)
  {
    renderWindow_->SetSize(width, height);
  }

  if (renderer_)
  {
    renderer_->SetViewport(0.0, 0.0, 1.0, 1.0);
  }

  if (overlayManager_)
  {
    overlayManager_->resize(static_cast<int>(width), static_cast<int>(height));
  }
}

void VtkRenderer::mousePress(float x, float y, MouseButton btn)
{
  cameraController_.mousePress(x, y, btn);
}

void VtkRenderer::mouseMove(float x, float y, MouseButton btn)
{
  cameraController_.mouseMove(x, y);

  renderFrame();
}

void VtkRenderer::mouseRelease()
{
  cameraController_.mouseRelease();
}

void VtkRenderer::mouseWheel(int32_t delta)
{
  cameraController_.mouseWheel(delta);

  renderFrame();
}

void VtkRenderer::autoView()
{
  renderer_->ResetCamera();

  renderFrame();
}

// Data----------------------------------------------------------------------------------------------
void VtkRenderer::updateGeometries(const std::vector<VtkGeometryObjectHandle>& geometries)
{
  if (!initialized_) initialize();

  renderer_->RemoveAllViewProps();
  actors_.clear();

  surfaceMappers_.clear();
  volumeMappers_.clear();

  for (const auto& geo : geometries)
  {
    addGeometry(geo);
  }

  if (first_update_)
  {
    renderer_->ResetCamera();
    first_update_ = false;
  }
}

void VtkRenderer::updateClippingPlanes(const std::vector<Core::Datatypes::ClippingPlane>& planes)
{
  clippingPlanes_ = planes;

  rebuildClippingPlanes();

  applyClippingPlanesToScene();

  renderFrame();
}

void VtkRenderer::setOrientationAxesVisible(bool visible)
{
  if (overlayManager_)
  {
    overlayManager_->setVisible(OverlayType::Orientation, visible);
    renderFrame();
  }
}

void VtkRenderer::setOrientationAxesSize(int size)
{
  if (overlayManager_)
  {
    if (auto orientationOverlay = overlayManager_->overlay<VtkOrientationOverlay>())
    {
      orientationOverlay->setSize(size);
      renderFrame();
    }
  }
}

void VtkRenderer::setOrientationAxesPosX(int x)
{
  if (overlayManager_)
  {
    if (auto orientationOverlay = overlayManager_->overlay<VtkOrientationOverlay>())
    {
      orientationOverlay->setPosX(x);
      renderFrame();
    }
  }
}

void VtkRenderer::setOrientationAxesPosY(int y)
{
  if (overlayManager_)
  {
    if (auto orientationOverlay = overlayManager_->overlay<VtkOrientationOverlay>())
    {
      orientationOverlay->setPosY(y);
      renderFrame();
    }
  }
}

void VtkRenderer::setSbVisible(bool visible)
{
  if (overlayManager_)
  {
    overlayManager_->setVisible(OverlayType::Scalebar, visible);
    renderFrame();
  }
}

void VtkRenderer::setSbFontSize(int size)
{
  if (overlayManager_)
  {
    if (auto scaleBarOverlay = overlayManager_->overlay<VtkScaleBarOverlay>())
    {
      scaleBarOverlay->setFontSize(size);
      renderFrame();
    }
  }
}

void VtkRenderer::setSbLength(double length)
{
  if (overlayManager_)
  {
    if (auto scaleBarOverlay = overlayManager_->overlay<VtkScaleBarOverlay>())
    {
      scaleBarOverlay->setLength(length);
      renderFrame();
    }
  }
}

void VtkRenderer::setSbHeight(double height)
{
  if (overlayManager_)
  {
    if (auto scaleBarOverlay = overlayManager_->overlay<VtkScaleBarOverlay>())
    {
      scaleBarOverlay->setHeight(height);
      renderFrame();
    }
  }
}

void VtkRenderer::setSbMultiplier(double mul)
{
  if (overlayManager_)
  {
    if (auto scaleBarOverlay = overlayManager_->overlay<VtkScaleBarOverlay>())
    {
      scaleBarOverlay->setMultiplier(mul);
      renderFrame();
    }
  }
}

void VtkRenderer::setSbNumTicks(double num)
{
  if (overlayManager_)
  {
    if (auto scaleBarOverlay = overlayManager_->overlay<VtkScaleBarOverlay>())
    {
      scaleBarOverlay->setNumTicks(num);
      renderFrame();
    }
  }
}

void VtkRenderer::setSbLineWidth(double width)
{
  if (overlayManager_)
  {
    if (auto scaleBarOverlay = overlayManager_->overlay<VtkScaleBarOverlay>())
    {
      scaleBarOverlay->setLineWidth(width);
      renderFrame();
    }
  }
}

void VtkRenderer::setSbLineColor(double color)
{
  if (overlayManager_)
  {
    if (auto scaleBarOverlay = overlayManager_->overlay<VtkScaleBarOverlay>())
    {
      scaleBarOverlay->setLineColor(color);
      renderFrame();
    }
  }
}

void VtkRenderer::setSbUnit(const std::string& unit)
{
  if (overlayManager_)
  {
    if (auto scaleBarOverlay = overlayManager_->overlay<VtkScaleBarOverlay>())
    {
      scaleBarOverlay->setUnit(unit);
      renderFrame();
    }
  }
}

void VtkRenderer::setSbProjLength(double length)
{
  if (overlayManager_)
  {
    if (auto scaleBarOverlay = overlayManager_->overlay<VtkScaleBarOverlay>())
    {
      scaleBarOverlay->setProjLength(length);
      renderFrame();
    }
  }
}

void VtkRenderer::saveScreenshot(const QString& filename)
{
  if (image_.isNull()) return;
  image_.save(filename);
}

void VtkRenderer::rebuildClippingPlanes()
{
  vtkClippingPlanes_.clear();

  double bounds[6];
  renderer_->ComputeVisiblePropBounds(bounds);

  double sx = bounds[1] - bounds[0];
  double sy = bounds[3] - bounds[2];
  double sz = bounds[5] - bounds[4];

  double cx = 0.5 * (bounds[0] + bounds[1]);
  double cy = 0.5 * (bounds[2] + bounds[3]);
  double cz = 0.5 * (bounds[4] + bounds[5]);

  double sceneDiag = std::sqrt(sx * sx + sy * sy + sz * sz);

  for (const auto& clip : clippingPlanes_)
  {
    if (!clip.visible) continue;

    auto plane = vtkSmartPointer<vtkPlane>::New();

    double nx = clip.x;
    double ny = clip.y;
    double nz = clip.z;

    double len = std::sqrt(nx * nx + ny * ny + nz * nz);

    if (len < 1e-10)
    {
      nx = 1.0;
      ny = 0.0;
      nz = 0.0;
    }
    else
    {
      nx /= len;
      ny /= len;
      nz /= len;
    }

    if (clip.reverseNormal)
    {
      nx = -nx;
      ny = -ny;
      nz = -nz;
    }

    double worldD = clip.d * 0.5 * sceneDiag;

    plane->SetNormal(nx, ny, nz);

    plane->SetOrigin(cx + nx * worldD, cy + ny * worldD, cz + nz * worldD);

    vtkClippingPlanes_.push_back(plane);
  }
}

void VtkRenderer::applyClippingPlanesToScene()
{
  for (auto& mapper : surfaceMappers_)
  {
    mapper->RemoveAllClippingPlanes();

    for (auto& plane : vtkClippingPlanes_)
      mapper->AddClippingPlane(plane);
  }

  for (auto& mapper : volumeMappers_)
  {
    mapper->RemoveAllClippingPlanes();

    for (auto& plane : vtkClippingPlanes_)
      mapper->AddClippingPlane(plane);
  }
}

void VtkRenderer::setBackgroundColor(const QColor& color)
{
  bgColor_ = color;
  renderer_->SetBackground(bgColor_.redF(), bgColor_.greenF(), bgColor_.blueF());
}

void VtkRenderer::initialize()
{
  renderWindow_ = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow_->SetOffScreenRendering(1);

  renderer_ = vtkSmartPointer<vtkRenderer>::New();

  renderWindow_->AddRenderer(renderer_);

  renderer_->SetBackground(bgColor_.redF(), bgColor_.greenF(), bgColor_.blueF());

  w2i_ = vtkSmartPointer<vtkWindowToImageFilter>::New();
  w2i_->SetInput(renderWindow_);

  // overlay
  overlayManager_ = std::make_unique<VtkOverlayManager>();
  // orientation axes
  auto orientationOverlay = std::make_unique<VtkOrientationOverlay>();
  orientationOverlay->setVisible(true);
  overlayManager_->addOverlay(OverlayType::Orientation, std::move(orientationOverlay));
  //scale bar
  auto scalebarOverlay = std::make_unique<VtkScaleBarOverlay>();
  scalebarOverlay->setVisible(true);
  overlayManager_->addOverlay(OverlayType::Scalebar, std::move(scalebarOverlay));
  //
  overlayManager_->resize(static_cast<int>(width_), static_cast<int>(height_));
  overlayManager_->initialize(renderer_);

  // camera
  vtkCamera* camera = renderer_->GetActiveCamera();
  cameraController_.setCamera(camera);

  cameraObserver_ = vtkSmartPointer<vtkCallbackCommand>::New();

  cameraObserver_->SetClientData(this);

  cameraObserver_->SetCallback([](vtkObject*, unsigned long, void* clientData, void*) {
    auto self = static_cast<VtkRenderer*>(clientData);

    self->onCameraModified();
  });

  camera->AddObserver(vtkCommand::ModifiedEvent, cameraObserver_);

  overlayManager_->cameraChanged(camera);

  initialized_ = true;
}

void VtkRenderer::onCameraModified()
{
  vtkCamera* camera = renderer_->GetActiveCamera();

  overlayManager_->cameraChanged(camera);
}

void VtkRenderer::addGeometry(const VtkGeometryObjectHandle& geo)
{
  if (!geo) return;

  auto poly = vtkPolyData::SafeDownCast(geo->dataObject);

  if (poly)
  {
    switch (geo->type)
    {
    case GeometryType::SPHERE: renderSpheres(poly, geo); return;

    case GeometryType::CYLINDER:
    case GeometryType::EDGE: renderCylinders(poly, geo); return;

    case GeometryType::STREAMLINE: renderStreamlines(poly, geo); return;

    default: break;
    }
  }

  if (!geo->dataObject) return;

  if (auto image = vtkImageData::SafeDownCast(geo->dataObject))
  {
    renderImageData(image, geo);
    return;
  }

  if (auto grid = vtkUnstructuredGrid::SafeDownCast(geo->dataObject))
  {
    renderUnstructuredGrid(grid, geo);
    return;
  }

  if (poly)
  {
    renderPolyData(poly, geo);
    return;
  }

  std::cout << "Unsupported VTK dataset" << std::endl;
}

void VtkRenderer::renderPolyData(vtkPolyData* poly, const VtkGeometryObjectHandle& geo)
{
  if (!poly) return;

  double range[2];
  poly->GetScalarRange(range);

  auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

  mapper->SetInputData(poly);

  auto actor = vtkSmartPointer<vtkActor>::New();

  if (geo->tfn.fromColorMap)
  {
    mapper->ScalarVisibilityOn();
    mapper->SetColorModeToMapScalars();
    mapper->SetScalarModeToUsePointData();
    mapper->SetScalarRange(range);

    auto lut = createLookupTable(geo->tfn, range);

    mapper->SetLookupTable(lut);
  }
  else
  {
    mapper->ScalarVisibilityOff();

    applyMaterial(actor, geo->material);
  }

  surfaceMappers_.push_back(mapper);

  applyCurrentClippingPlanes(mapper.Get());

  actor->SetMapper(mapper);

  renderer_->AddActor(actor);

  actors_.push_back(actor);
}

void VtkRenderer::renderUnstructuredGrid(vtkUnstructuredGrid* ugrid, const VtkGeometryObjectHandle& geo)
{
  if (!ugrid) return;

  double range[2];
  ugrid->GetScalarRange(range);

  auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();

  mapper->SetInputData(ugrid);

  surfaceMappers_.push_back(mapper);

  applyCurrentClippingPlanes(mapper.Get());

  auto actor = vtkSmartPointer<vtkActor>::New();

  if (geo->tfn.fromColorMap)
  {
    mapper->ScalarVisibilityOn();
    mapper->SetColorModeToMapScalars();
    mapper->SetScalarModeToUsePointData();
    mapper->SetScalarRange(range);

    auto lut = createLookupTable(geo->tfn, range);

    mapper->SetLookupTable(lut);
  }
  else
  {
    mapper->ScalarVisibilityOff();

    applyMaterial(actor, geo->material);
  }

  actor->SetMapper(mapper);

  applyMaterial(actor, geo->material);

  renderer_->AddActor(actor);

  actors_.push_back(actor);
}

void VtkRenderer::renderImageData(vtkImageData* image, const VtkGeometryObjectHandle& geo)
{
  auto mapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
  mapper->SetInputData(image);

  volumeMappers_.push_back(mapper);

  applyCurrentClippingPlanes(mapper.Get());

  auto volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  volumeProperty->ShadeOff();
  volumeProperty->SetInterpolationTypeToLinear();

  //----------------------------------
  // Opacity transfer function
  //----------------------------------

  auto opacity = vtkSmartPointer<vtkPiecewiseFunction>::New();

  //----------------------------------
  // Color transfer function
  //----------------------------------

  auto color = vtkSmartPointer<vtkColorTransferFunction>::New();

  if (geo->tfn.fromColorMap)
  {
    double minVal = geo->tfn.range[0];
    double maxVal = geo->tfn.range[1];

    const auto& colors = geo->tfn.colors;
    const auto& opacities = geo->tfn.opacities;

    const size_t n = std::min(colors.size() / 3, opacities.size());

    if (n == 1)
    {
      color->AddRGBPoint(minVal, colors[0], colors[1], colors[2]);

      color->AddRGBPoint(maxVal, colors[0], colors[1], colors[2]);

      opacity->AddPoint(minVal, opacities[0]);
      opacity->AddPoint(maxVal, opacities[0]);
    }
    else
    {
      for (size_t i = 0; i < n; ++i)
      {
        double t = static_cast<double>(i) / (n - 1);
        double scalar = minVal + t * (maxVal - minVal);

        color->AddRGBPoint(scalar, colors[3 * i], colors[3 * i + 1], colors[3 * i + 2]);

        opacity->AddPoint(scalar, opacities[i]);
      }
    }
  }
  else
  {
    double range[2];
    image->GetScalarRange(range);

    color->AddRGBPoint(range[0], geo->tfn.colors[0], geo->tfn.colors[1], geo->tfn.colors[2]);

    color->AddRGBPoint(range[1], geo->tfn.colors[0], geo->tfn.colors[1], geo->tfn.colors[2]);

    opacity->AddPoint(range[0], 0.0);
    opacity->AddPoint(range[1], geo->tfn.opacities[0]);
  }

  volumeProperty->SetScalarOpacity(opacity);
  volumeProperty->SetColor(color);

  auto volume = vtkSmartPointer<vtkVolume>::New();
  volume->SetMapper(mapper);
  volume->SetProperty(volumeProperty);

  renderer_->AddVolume(volume);
}

void VtkRenderer::renderCylinders(vtkPolyData* poly, const VtkGeometryObjectHandle& geo)
{
  if (!poly) return;

  auto tube = vtkSmartPointer<vtkTubeFilter>::New();
  tube->SetInputData(poly);
  tube->SetRadius(geo->radius);
  tube->SetNumberOfSides(20);
  tube->CappingOn();
  tube->Update();

  double range[2];
  poly->GetScalarRange(range);

  auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(tube->GetOutputPort());
  auto actor = vtkSmartPointer<vtkActor>::New();

  if (geo->tfn.fromColorMap)
  {
    mapper->ScalarVisibilityOn();
    mapper->SetScalarRange(range);
    mapper->SetLookupTable(createLookupTable(geo->tfn, range));
  }
  else
  {
    mapper->ScalarVisibilityOff();
    applyMaterial(actor, geo->material);
  }

  surfaceMappers_.push_back(mapper);

  applyCurrentClippingPlanes(mapper.Get());

  actor->SetMapper(mapper);

  renderer_->AddActor(actor);
  actors_.push_back(actor);
}

void VtkRenderer::renderStreamlines(vtkPolyData* poly, const VtkGeometryObjectHandle& geo)
{
  if (!poly) return;

  auto tube = vtkSmartPointer<vtkTubeFilter>::New();
  tube->SetInputData(poly);
  tube->SetRadius(geo->radius);
  tube->SetNumberOfSides(16);
  tube->CappingOff();
  tube->Update();

  double range[2];
  poly->GetScalarRange(range);

  auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(tube->GetOutputPort());
  auto actor = vtkSmartPointer<vtkActor>::New();

  if (geo->tfn.fromColorMap)
  {
    mapper->ScalarVisibilityOn();
    mapper->SetScalarRange(range);
    mapper->SetLookupTable(createLookupTable(geo->tfn, range));
  }
  else
  {
    mapper->ScalarVisibilityOff();
    applyMaterial(actor, geo->material);
  }

  surfaceMappers_.push_back(mapper);

  applyCurrentClippingPlanes(mapper.Get());

  actor->SetMapper(mapper);

  renderer_->AddActor(actor);
  actors_.push_back(actor);
}

void VtkRenderer::renderSpheres(vtkPolyData* poly, const VtkGeometryObjectHandle& geo)
{
  if (!poly) return;

  auto sphere = vtkSmartPointer<vtkSphereSource>::New();
  sphere->SetRadius(geo->radius);
  sphere->SetThetaResolution(16);
  sphere->SetPhiResolution(16);

  double range[2];
  poly->GetScalarRange(range);

  auto mapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
  mapper->SetInputData(poly);
  mapper->SetSourceConnection(sphere->GetOutputPort());
  auto actor = vtkSmartPointer<vtkActor>::New();

  if (geo->tfn.fromColorMap)
  {
    mapper->ScalarVisibilityOn();
    mapper->SetScalarRange(range);
    mapper->SetLookupTable(createLookupTable(geo->tfn, range));
  }
  else
  {
    mapper->ScalarVisibilityOff();
    applyMaterial(actor, geo->material);
  }

  surfaceMappers_.push_back(mapper);

  applyCurrentClippingPlanes(mapper.Get());

  actor->SetMapper(mapper);

  renderer_->AddActor(actor);
  actors_.push_back(actor);
}

void VtkRenderer::applyMaterial(vtkActor* actor, const VtkGeometryObject::Material& mat)
{
  if (!actor) return;

  auto prop = actor->GetProperty();

  prop->SetColor(mat.color[0], mat.color[1], mat.color[2]);

  prop->SetOpacity(mat.opacity);
}

vtkSmartPointer<vtkLookupTable> VtkRenderer::createLookupTable(const VtkGeometryObject::TransferFunc& tfn, double range[2])
{
  auto lut = vtkSmartPointer<vtkLookupTable>::New();

  if (tfn.colors.empty())
  {
    lut->SetRange(range);
    lut->SetHueRange(0.667, 0.0);
    lut->Build();
    return lut;
  }

  const size_t n = std::min(tfn.colors.size() / 3, tfn.opacities.empty() ? tfn.colors.size() / 3 : tfn.opacities.size());

  lut->SetNumberOfTableValues(static_cast<vtkIdType>(n));
  lut->SetRange(range);

  for (size_t i = 0; i < n; ++i)
  {
    double a = 1.0;

    if (i < tfn.opacities.size()) a = tfn.opacities[i];

    lut->SetTableValue(static_cast<vtkIdType>(i), tfn.colors[3 * i], tfn.colors[3 * i + 1], tfn.colors[3 * i + 2], a);
  }

  lut->Build();

  return lut;
}

void VtkRenderer::addDirectionalLight(glm::vec3 color, glm::vec3 direction) {}

void VtkRenderer::addAmbientLight(glm::vec3 color, float intensity) {}

#endif
