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

#include <unordered_map>
#include <Core/Datatypes/Feedback.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun
{
  namespace Graphics
  {
    namespace Datatypes
    {
      struct WidgetInfo
      {
        WidgetInfo(Core::Datatypes::WidgetMovement move) : moveType(move) {}
        Core::Datatypes::WidgetMovement moveType;
        glm::mat4 scaleTrans;
        glm::vec3 origin;
        glm::vec3 scaleAxis;
        glm::vec3 flipAxis;
        glm::vec3 translationAxis;
        bool flipInvertedWidget;
        bool negate;
        int scaleAxisIndex;
      };

      class SCISHARE WidgetBase : public GeometryObjectSpire {
      public:
        WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag,
                   bool isClippable);
        WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag,
                   bool isClippable, const Core::Geometry::Point& origin);
        WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag,
                   bool isClippable, const Core::Geometry::Point& pos,
                   const Core::Geometry::Point& origin);
        Core::Geometry::Point position() const;
        void setPosition(const Core::Geometry::Point& p);
        void setToTranslationAxis(Core::Datatypes::MouseButton btn, const Core::Geometry::Vector& v);
        void setToScale(Core::Datatypes::MouseButton btn, const Core::Geometry::Vector& flipAxis, bool negate);
        void setToScale(Core::Datatypes::MouseButton btn, bool negate);
        void setToScaleAxis(Core::Datatypes::MouseButton btn, const Core::Geometry::Vector& scaleAxis,
                            const Core::Geometry::Vector& flipAxis, glm::mat4 scaleTrans,
                            int scaleAxisIndex, bool negate);
        void setToScaleAxis(Core::Datatypes::MouseButton btn, const Core::Geometry::Vector& scaleAxis,
                            glm::mat4 scaleTrans, int scaleAxisIndex, bool negate);
        void setToScaleUnidirectional(Core::Datatypes::MouseButton btn, const Core::Geometry::Vector& translationAxis,
                                      const Core::Geometry::Vector& flipAxis, bool negate);
        void setToScaleUnidirectional(Core::Datatypes::MouseButton btn, const Core::Geometry::Vector& translationAxis,
                                      bool negate);
        void setToScaleAxisUnidirectional(Core::Datatypes::MouseButton btn, const Core::Geometry::Vector& scaleAxis,
                                          const Core::Geometry::Vector& flipAxis, glm::mat4 scaleTrans,
                                          int scaleAxisIndex, bool negate);
        void setToScaleAxisUnidirectional(Core::Datatypes::MouseButton btn, const Core::Geometry::Vector& scaleAxis,
                                          glm::mat4 scaleTrans, int scaleAxisIndex, bool negate);
        void setToScaleAxisHalf(Core::Datatypes::MouseButton btn, const Core::Geometry::Vector& scaleAxis,
                                const Core::Geometry::Vector& flipAxis, glm::mat4 scaleTrans,
                                int scaleAxisIndex, bool negate);
        void setToScaleAxisHalf(Core::Datatypes::MouseButton btn, const Core::Geometry::Vector& scaleAxis,
                                glm::mat4 scaleTrans, int scaleAxisIndex, bool negate);
        void setToRotate(Core::Datatypes::MouseButton btn);
        void setToTranslate(Core::Datatypes::MouseButton btn);
        void addMovementMap(Core::Datatypes::WidgetMovement key,
                            std::pair<Core::Datatypes::WidgetMovement, std::vector<std::string> > moves);
        glm::vec3 getFlipVector();
        glm::vec3 getScaleVector();
        glm::vec3 getTranslationVector();
        const glm::mat4 getScaleTransform();
        int getScaleAxisIndex();
        bool getFlipInvertedWidget();
        std::vector<WidgetInfo> getMovementInfo();
        glm::vec3 translationAxis_;

        glm::vec3 origin_;
        std::unordered_map<Core::Datatypes::WidgetMovement,
                           std::vector<std::pair<Core::Datatypes::WidgetMovement,
                                                 std::vector<std::string>>>,
                           Core::Datatypes::EnumClassHash> moveMap_;
        void addInitialId();

      protected:
        Core::Geometry::Point position_;

      private:
        std::vector<WidgetInfo> movementInfo_;
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
          : WidgetBase(idGenerator, tag, true), widgets_(begin, end)
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
        static WidgetHandle createWidgetComposite(const Core::GeometryIDGenerator& idGenerator,
                                                  const std::string& tag, WidgetIter begin, WidgetIter end)
      {
        return boost::make_shared<CompositeWidget>(idGenerator, tag, begin, end);
      }
    }
  }
}

#endif
