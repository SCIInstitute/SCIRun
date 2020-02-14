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
      class SCISHARE WidgetBase : public GeometryObjectSpire
      {
      public:
        WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable);
        WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable, const Core::Geometry::Point& origin);
        WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable, const Core::Geometry::Point& pos, const Core::Geometry::Point& origin);
        Core::Geometry::Point position() const;
        void setPosition(const Core::Geometry::Point& p);
        void setToScale(const Core::Geometry::Vector& flipAxis);
        void setToRotate();
        void setToTranslate();
        glm::vec3 getFlipVector();
        WidgetMovement getMovementType();

        glm::vec3 origin_;
        std::vector<std::string> connectedIds_;
        void addInitialId();

      protected:
        Core::Geometry::Point position_;
      private:
        WidgetMovement movementType_;
        glm::vec3 flipAxis_;
      };

        using WidgetHandle = SharedPointer<WidgetBase>;

        struct SCISHARE BoxPosition {
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
        ~CompositeWidget();
        void addToList(Core::Datatypes::GeometryBaseHandle handle, Core::Datatypes::GeomList& list) override;
        void addToList(WidgetHandle handle);
        std::vector<std::string> getListOfConnectedIds();

        std::vector<WidgetHandle> widgets_;
      private:
      };

      using CompositeWidgetHandle = SharedPointer<CompositeWidget>;

      template <typename WidgetIter>
        static WidgetHandle createWidgetComposite(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, WidgetIter begin, WidgetIter end)
      {
        return boost::make_shared<CompositeWidget>(idGenerator, tag, begin, end);
      }
    }
  }
}

#endif
