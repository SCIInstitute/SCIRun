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

#ifndef Graphics_Graphics_Widgets_Widget_H
#define Graphics_Graphics_Widgets_Widget_H

#include <Graphics/Datatypes/GeometryImpl.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun
{
  namespace Graphics
  {
    namespace Datatypes
    {

      class SCISHARE WidgetBase : public GeometryObjectSpire
      {
      public:
        WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable);
      };

      using WidgetHandle = SharedPointer<WidgetBase>;

      struct SCISHARE BoxPosition
      {
        Core::Geometry::Point center_, right_, down_, in_;

        void setPosition(const Core::Geometry::Point& center, const Core::Geometry::Point& right,
          const Core::Geometry::Point& down, const Core::Geometry::Point& in);
        void getPosition(Core::Geometry::Point& center, Core::Geometry::Point& right,
          Core::Geometry::Point& down, Core::Geometry::Point& in) const;
      };

      class SCISHARE CompositeWidget : public WidgetBase
      {
      public:
        template <typename WidgetIter>
        CompositeWidget(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, WidgetIter begin, WidgetIter end)
          : WidgetBase(idGenerator, tag, true), widgets_(begin, end)
        {}
        ~CompositeWidget();
        void addToList(Core::Datatypes::GeometryBaseHandle handle, Core::Datatypes::GeomList& list) override;
      private:
        std::vector<WidgetHandle> widgets_;
      };

      class SCISHARE LinkedCompositeWidget : public WidgetBase
      {
      public:
        template <typename WidgetIter>
          LinkedCompositeWidget(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, WidgetIter begin, WidgetIter end)
          : WidgetBase(idGenerator, tag, true), widgets_(begin, end)
          {}
        ~LinkedCompositeWidget();
        void addToList(Core::Datatypes::GeometryBaseHandle handle, Core::Datatypes::GeomList& list) override;
      private:
        std::vector<WidgetHandle> widgets_;
      };

      class SCISHARE WidgetFactory
      {
      public:
      static WidgetHandle createBox(const Core::GeometryIDGenerator& idGenerator, double scale,
                                    const BoxPosition& pos, const Core::Geometry::BBox& bbox);
      static WidgetHandle createSphere(const Core::GeometryIDGenerator& idGenerator,
                                       const std::string& name,
                                       double radius, const std::string& defaultColor,
                                       const Core::Geometry::Point& point, const Core::Geometry::BBox& bbox);
      static WidgetHandle createCylinder(const Core::GeometryIDGenerator& idGenerator,
                                         const std::string& name,
                                         double scale,
                                         const std::string& defaultColor,
                                         const Core::Geometry::Point& p1,
                                         const Core::Geometry::Point& p2,
                                         const Core::Geometry::BBox& bbox);
      static WidgetHandle createCone(const Core::GeometryIDGenerator& idGenerator,
                                     const std::string& name,
                                     double scale,
                                     const std::string& defaultColor,
                                     const Core::Geometry::Point& p1,
                                     const Core::Geometry::Point& p2,
                                     const Core::Geometry::BBox& bbox);
       template <typename WidgetIter>
        static WidgetHandle createComposite(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, WidgetIter begin, WidgetIter end)
        {
          return boost::make_shared<CompositeWidget>(idGenerator, tag, begin, end);
        }

       template <typename WidgetIter>
       static WidgetHandle createLinkedComposite(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, WidgetIter begin, WidgetIter end)
       {
         return boost::make_shared<LinkedCompositeWidget>(idGenerator, tag, begin, end);
       }
      };
    }
  }
}

#endif
