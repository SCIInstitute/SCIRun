/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Graphics/Widgets/WidgetFactory.h>
#include <Graphics/Widgets/ArrowWidget.h>
#include <Graphics/Widgets/BoundingBoxWidget.h>
#include <Graphics/Widgets/ConeWidget.h>
#include <Graphics/Widgets/CylinderWidget.h>
#include <Graphics/Widgets/DiskWidget.h>
#include <Graphics/Widgets/SphereWidget.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;

WidgetHandle WidgetFactory::createArrowWidget(const Core::GeometryIDGenerator &idGenerator,
                                                       const std::string& name,
                                                       ArrowParameters params)
{
  return boost::make_shared<ArrowWidget>(idGenerator, name, params);
}

WidgetHandle WidgetFactory::createBox(const Core::GeometryIDGenerator& idGenerator,
                                               BasicBoundingBoxParameters params)
{
  auto widget = boost::make_shared<BasicBoundingBoxWidget>(idGenerator, params);
  //widget->addInitialId();
  //widget->setToTranslate();
  return widget;
}

WidgetHandle WidgetFactory::createSphere(const Core::GeometryIDGenerator& idGenerator,
                                         const std::string& name,
                                         SphereParameters params)
{
  auto widget = boost::make_shared<SphereWidget>(idGenerator, name, params);
  //widget->addInitialId();
  //widget->setToTranslate();
  return widget;
}

WidgetHandle WidgetFactory::createCylinder(const Core::GeometryIDGenerator& idGenerator,
                                           const std::string& name,
                                           CylinderParameters params)
{
  auto widget = boost::make_shared<CylinderWidget>(idGenerator, name, params);
  // widget->addInitialId();
  // widget->setToTranslate();
  return widget;
}

WidgetHandle WidgetFactory::createCone(const Core::GeometryIDGenerator& idGenerator,
                                       const std::string& name,
                                       ConeParameters params)
{
  auto widget = boost::make_shared<ConeWidget>(idGenerator, name, params);
  // widget->addInitialId();
  // widget->setToTranslate();
  return widget;
}

WidgetHandle WidgetFactory::createDisk(const Core::GeometryIDGenerator& idGenerator,
                                       const std::string& name,
                                       DiskParameters params)
{
  auto widget = boost::make_shared<DiskWidget>(idGenerator, name, params);
  // widget->addInitialId();
  // widget->setToTranslate();
  return widget;
}
