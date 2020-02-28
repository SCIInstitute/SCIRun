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


#ifndef Graphics_Graphics_Widgets_Widget_H
#define Graphics_Graphics_Widgets_Widget_H

#include <Core/GeometryPrimitives/Point.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/WidgetParameters.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun
{
  namespace Graphics
  {
    namespace Datatypes
    {
      class WidgetBase;

      struct SCISHARE GeometryIdGetter
      {
        std::string operator()(const GeometryObjectSpire* w) const { return w->uniqueID(); }
      };

      using WidgetObservable = Observable<WidgetBase*, WidgetMovement, SimpleWidgetEvent,
        SimpleWidgetEventKey, SimpleWidgetEventValue, GeometryIdGetter>;

      class SCISHARE WidgetBase : public GeometryObjectSpire,
        public WidgetObservable,
        public InputTransformMapper,
        public Transformable
      {
      public:
        explicit WidgetBase(const WidgetBaseParameters& params);
        WidgetBase(const WidgetBase&) = delete;

        Core::Geometry::Point position() const;
        void setPosition(const Core::Geometry::Point& p);

        const std::string& name() const { return name_; }

        virtual void propagateEvent(const SimpleWidgetEvent& e);
      protected:
        Core::Geometry::Point position_;
        std::string name_;
      };

      using WidgetHandle = SharedPointer<WidgetBase>;
      using WidgetList = std::vector<WidgetHandle>;
      using WidgetListIterator = WidgetList::const_iterator;

      class SCISHARE CompositeWidget : public WidgetBase
      {
      public:
        template <typename WidgetIter>
        CompositeWidget(const WidgetBaseParameters& params, WidgetIter begin, WidgetIter end)
          : WidgetBase(params), widgets_(begin, end)
        {}
        explicit CompositeWidget(const WidgetBaseParameters& params) : WidgetBase(params)
        {}

        void addToList(Core::Datatypes::GeometryBaseHandle handle, Core::Datatypes::GeomList& list) override;
        void propagateEvent(const SimpleWidgetEvent& e) override;
        WidgetListIterator subwidgetBegin() const { return widgets_.begin(); }
        WidgetListIterator subwidgetEnd() const { return widgets_.end(); }

      protected:
        WidgetList widgets_;
        void registerAllSiblingWidgetsForEvent(WidgetHandle selected, WidgetMovement movement);
      };

      using CompositeWidgetHandle = SharedPointer<CompositeWidget>;
    }
  }
}

#endif
