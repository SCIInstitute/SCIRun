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

#ifndef Graphics_Graphics_Widgets_WidgetFactory_H
#define Graphics_Graphics_Widgets_WidgetFactory_H

#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/Widget.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun {
  namespace Graphics {
    namespace Datatypes {
    class SCISHARE WidgetFactory {
    public:
      static WidgetHandle createArrowWidget(const Core::GeometryIDGenerator &idGenerator,
                                            const std::string &name, ArrowParameters params);

      static WidgetHandle createBox(const Core::GeometryIDGenerator &idGenerator,
                                    BasicBoundingBoxParameters params);

      static WidgetHandle createSphere(const Core::GeometryIDGenerator &idGenerator,
                                       const std::string &name, SphereParameters params);

      static WidgetHandle createCylinder(const Core::GeometryIDGenerator &idGenerator,
                                         const std::string &name, CylinderParameters params);

      static WidgetHandle createCone(const Core::GeometryIDGenerator &idGenerator,
                                     const std::string &name, ConeParameters params);

      static WidgetHandle createDisk(const Core::GeometryIDGenerator &idGenerator,
                                     const std::string &name, DiskParameters params);

      template <typename WidgetIter>
      static CompositeWidgetHandle
      createComposite(const Core::GeometryIDGenerator &idGenerator,
                      const std::string &tag, WidgetIter begin,
                      WidgetIter end) {
        return boost::make_shared<CompositeWidget>(idGenerator, tag, begin,
                                                   end);
      }
    };
    } // namespace Datatypes
  } // namespace Graphics
} // namespace SCIRun

#endif
