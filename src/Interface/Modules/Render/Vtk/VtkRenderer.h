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

#pragma once

#include <vector>
#include <cstdio>


#ifdef WITH_VTK
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWindowToImageFilter.h>
#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

#include "VtkCameraController.h"

#include <Core/Datatypes/Feedback.h>
#include <Core/Datatypes/VtkGeometry.h>
#include <Interface/Modules/Render/Vtk/share.h>
#include <glm/glm.hpp>
#include <QImage>

namespace SCIRun { namespace Render {

class SCISHARE VtkRenderer
{
public:
  VtkRenderer();
  virtual ~VtkRenderer();

  //Rendering---------------------------------------------------------------------------------------
  void renderFrame(); //renders frame with current OpenGL context

  //Interaction-------------------------------------------------------------------------------------
  void resize(uint32_t width, uint32_t height);
  void mousePress(float x, float y, MouseButton btn);
  void mouseMove(float x, float y, MouseButton btn);
  void mouseRelease();
  void mouseWheel(int32_t delta);
  void autoView();

  //Data--------------------------------------------------------------------------------------------
  void updateGeometries(const std::vector<Core::Datatypes::VtkGeometryObjectHandle>& geometries);

  //Getters-----------------------------------------------------------------------------------------
  uint32_t width() {return width_;}
  uint32_t height() {return height_;}
  const unsigned char* imagePixels() const { return imagePixels_; }
  QImage image() const { return image_; }

 private:
  static int vtkRendererInstances;

  uint32_t width_  {16};
  uint32_t height_ {16};
  uint32_t framesAccumulated {0};

  bool isScivis {true};

  void initialize();

    //----------------------------------------
  // Geometry dispatch
  //----------------------------------------
  void addGeometry(const Core::Datatypes::VtkGeometryObjectHandle& geo);

  //----------------------------------------
  // Dataset renderers
  //----------------------------------------
  void renderPolyData(vtkPolyData* poly, const Core::Datatypes::VtkGeometryObjectHandle& geo);
  void renderUnstructuredGrid(vtkUnstructuredGrid* grid, const Core::Datatypes::VtkGeometryObjectHandle& geo);
  void renderImageData(vtkImageData* image, const Core::Datatypes::VtkGeometryObjectHandle& geo);

  //----------------------------------------
  // Materials / transfer functions
  //----------------------------------------
  void applyMaterial(vtkActor* actor, const Core::Datatypes::VtkGeometryObject::Material& mat);
  vtkSmartPointer<vtkLookupTable> createLookupTable(const Core::Datatypes::VtkGeometryObject::TransferFunc& tfn, double range[2]);

  //----------------------------------------
  // Lighting
  //----------------------------------------
  void addDirectionalLight(glm::vec3 color, glm::vec3 direction);
  void addAmbientLight(glm::vec3 color, float intensity);

  bool initialized_ = false;
  vtkSmartPointer<vtkRenderer> renderer_;
  vtkSmartPointer<vtkRenderWindow> renderWindow_;
  vtkSmartPointer<vtkRenderWindowInteractor> interactor_;
  std::vector<vtkSmartPointer<vtkActor>> actors_;
  vtkSmartPointer<vtkWindowToImageFilter> w2i_;

  VtkCameraController cameraController_;

  unsigned char* imagePixels_ = nullptr;
  QImage image_;
};

}}

#endif
