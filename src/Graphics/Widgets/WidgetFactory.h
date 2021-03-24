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


#ifndef Graphics_Graphics_Widgets_WidgetFactory_H
#define Graphics_Graphics_Widgets_WidgetFactory_H

#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/Widget.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun
{
  namespace Graphics
  {
    namespace Datatypes
    {
      class SCISHARE WidgetFactory
      {
      public:
        static void setGlyphFactory(AbstractGlyphFactoryPtr glyphMaker);

        static WidgetHandle createArrowWidget(const WidgetBaseParameters& gen, ArrowParameters params);

        static WidgetHandle createBasicBoundingBox(const WidgetBaseParameters& gen,
                                                   BasicBoundingBoxParameters params);

        static WidgetHandle createBoundingBox(const WidgetBaseParameters& gen,
                                              BoundingBoxParameters params);

        static WidgetHandle createSphere(const WidgetBaseParameters& gen, SphereParameters params);

        static WidgetHandle createCylinder(const WidgetBaseParameters& gen, CylinderParameters params);

        static WidgetHandle createCone(const WidgetBaseParameters& gen, ConeParameters params);

        static WidgetHandle createDisk(const WidgetBaseParameters& gen, DiskParameters params);

        static WidgetHandle createSuperquadric(const WidgetBaseParameters& gen, SuperquadricParameters params);

        template <typename WidgetIter>
        static WidgetHandle createComposite(const WidgetBaseParameters& gen, WidgetIter begin, WidgetIter end)
        {
          return boost::make_shared<CompositeWidget>(gen, begin, end);
        }

      private:
        static AbstractGlyphFactoryPtr glyphMaker_;
        static GeneralWidgetParameters packageWithGlyph(const WidgetBaseParameters& params);
      };
    }
  }
}

#endif
