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


#include <Graphics/Widgets/WidgetFactory.h>
#include <Graphics/Widgets/ArrowWidget.h>
#include <Graphics/Widgets/BoundingBoxWidget.h>
#include <Graphics/Widgets/ConeWidget.h>
#include <Graphics/Widgets/CylinderWidget.h>
#include <Graphics/Widgets/DiskWidget.h>
#include <Graphics/Widgets/SphereWidget.h>
#include <Graphics/Widgets/GlyphFactory.h>
#include <Graphics/Widgets/WidgetBuilders.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;

AbstractGlyphFactoryPtr WidgetFactory::glyphMaker_(new RealGlyphFactory);

GeneralWidgetParameters WidgetFactory::packageWithGlyph(const WidgetBaseParameters& params)
{
  return {params, glyphMaker_};
}

void WidgetFactory::setGlyphFactory(AbstractGlyphFactoryPtr glyphMaker) { glyphMaker_ = glyphMaker; }

WidgetHandle WidgetFactory::createArrowWidget(const WidgetBaseParameters& gen,
                                                       ArrowParameters params)
{
  return boost::make_shared<ArrowWidget>(packageWithGlyph(gen), params);
}

WidgetHandle WidgetFactory::createBox(const WidgetBaseParameters& gen,
                                               BasicBoundingBoxParameters params)
{
  return boost::make_shared<BasicBoundingBoxWidget>(packageWithGlyph(gen), params);
}

WidgetHandle WidgetFactory::createSphere(const WidgetBaseParameters& gen,
                                         SphereParameters params)
{
  return boost::make_shared<SphereWidget>(packageWithGlyph(gen), params);
}

WidgetHandle WidgetFactory::createCylinder(const WidgetBaseParameters& gen,
                                           CylinderParameters params)
{
  return boost::make_shared<CylinderWidget>(packageWithGlyph(gen), params);
}

WidgetHandle WidgetFactory::createCone(const WidgetBaseParameters& gen,
                                       ConeParameters params)
{
  return boost::make_shared<ConeWidget>(packageWithGlyph(gen), params);
}

WidgetHandle WidgetFactory::createDisk(const WidgetBaseParameters& gen,
                                       DiskParameters params)
{
  return boost::make_shared<DiskWidget>(packageWithGlyph(gen), params);
}

WidgetHandle SphereWidgetBuilder::build() const
{
  return WidgetFactory::createSphere({ idGenerator_, tag_, mapping_ },
    { { scale_, defaultColor_, origin_, bbox_, resolution_ }, point_ });
}
