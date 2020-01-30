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

#include <Core/GeometryPrimitives/Point.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun
{
  namespace Graphics
  {
    namespace Datatypes
    {
      // These will give different types of widget movement through ViewScene.
      // To use rotation and scaling, an origin point must be given.
      enum class WidgetMovement
      {
        TRANSLATE,
        ROTATE,
        SCALE
      };

      class SCISHARE WidgetManipulationStuff
      {
        //void setToScale(const Core::Geometry::Vector& flipAxis);
        //void setToRotate();
        //void setToTranslate();
        //glm::vec3 getFlipVector();
        //WidgetMovement getMovementType();

        //void addInitialId();

        //WidgetMovement movementType_;
        //glm::vec3 flipAxis_;

        //glm::vec3 origin_;
        //std::vector<std::string> connectedIds_;
      };

      struct SCISHARE WidgetParameters
      {
        //TODO
      };

      class SCISHARE WidgetBase : public GeometryObjectSpire
      {
      public:
        WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable);
        // WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable, const Core::Geometry::Point& origin);
        // WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable, const Core::Geometry::Point& pos, const Core::Geometry::Point& origin);

        Core::Geometry::Point position() const;
        void setPosition(const Core::Geometry::Point& p);

      protected:
        Core::Geometry::Point position_;
      private:

      };

      using WidgetHandle = SharedPointer<WidgetBase>;

      struct SCISHARE BoxPosition
      {
        Core::Geometry::Point center_, right_, down_, in_;

        void setPosition(const Core::Geometry::Point &center,
                         const Core::Geometry::Point &right,
                         const Core::Geometry::Point &down,
                         const Core::Geometry::Point &in);
        void getPosition(Core::Geometry::Point &center,
                         Core::Geometry::Point &right,
                         Core::Geometry::Point &down,
                         Core::Geometry::Point &in) const;
      };

      class SCISHARE CompositeWidget : public WidgetBase
      {
      public:
        template <typename WidgetIter>
                CompositeWidget(const Core::GeometryIDGenerator& idGenerator, const std::string& tag,
                                WidgetIter begin, WidgetIter end)
                : WidgetBase(idGenerator, tag, true)
        {}
      CompositeWidget(const Core::GeometryIDGenerator& idGenerator, const std::string& tag)
                  : WidgetBase(idGenerator, tag, true)
        {}

        void addToList(Core::Datatypes::GeometryBaseHandle handle, Core::Datatypes::GeomList& list) override;
        void addToList(WidgetHandle handle);

      protected:
        std::vector<std::string> getListOfConnectedIds();
        std::vector<WidgetHandle> widgets_;
      };

      using CompositeWidgetHandle = SharedPointer<CompositeWidget>;

      template <typename WidgetIter>
        static WidgetHandle createWidgetComposite(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, WidgetIter begin, WidgetIter end)
      {
        return boost::make_shared<CompositeWidget>(idGenerator, tag, begin, end);
      }

      struct SCISHARE CommonWidgetParameters
      {
        double scale;
        std::string defaultColor;
        Core::Geometry::Point origin;
        Core::Geometry::BBox bbox;
        int resolution;
      };

      struct SCISHARE SphereParameters
      {
        CommonWidgetParameters common;
        Core::Geometry::Point point;
      };

      struct SCISHARE CylinderParameters
      {
        CommonWidgetParameters common;
        Core::Geometry::Point p1, p2;
      };

      using DiskParameters = CylinderParameters;

      struct SCISHARE ConeParameters
      {
        CylinderParameters cylinder;
        bool renderBase;
      };

      struct SCISHARE BasicBoundingBoxParameters
      {
        CommonWidgetParameters common;
        BoxPosition pos;
      };

      struct SCISHARE ArrowParameters
      {
        CommonWidgetParameters common;
        Core::Geometry::Point pos;
        Core::Geometry::Vector dir;
        bool show_as_vector;
        size_t widget_num, widget_iter;
      };
    }
  }
}

#endif
