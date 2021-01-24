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


#ifndef Graphics_Widgets_BoundingBoxWidget_H
#define Graphics_Widgets_BoundingBoxWidget_H

#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Graphics/Widgets/Widget.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun {
  namespace Graphics {
    namespace Datatypes {

      class SCISHARE BoundingBoxWidget : public CompositeWidget
      {
      public:
        BoundingBoxWidget(const GeneralWidgetParameters& gen, BoundingBoxParameters params);
      };

      class SCISHARE BBoxDataHandler
      {
      public:
      BBoxDataHandler(const Core::Geometry::Point& center,
                      const std::vector<Core::Geometry::Vector>& scaledEigvecs);
      void makeCylinders(const GeneralWidgetParameters& gen, const CommonWidgetParameters& params,
                         WidgetBase& widget);
      void makeCornerSpheres(const GeneralWidgetParameters& gen, const CommonWidgetParameters& params,
                             WidgetBase& widget);
      void makeFaceSpheres(const GeneralWidgetParameters& gen, const CommonWidgetParameters& params,
                           WidgetBase& widget);
      void makeFaceDisks(const GeneralWidgetParameters& gen, const CommonWidgetParameters& params,
                         WidgetBase& widget);
      glm::mat4 getScaleTrans() const;
      size_t getIndexOfDirectionOfFace(int f) const;
      Core::Geometry::Vector getDirectionOfFace(int f) const;
      std::vector<WidgetHandle> getEdges() const;
      std::vector<WidgetHandle> getCorners() const;
      std::vector<WidgetHandle> getFaceSpheres() const;
      std::vector<WidgetHandle> getFaceDisks() const;
      std::vector<WidgetHandle> getCornersOfFace(int f);
      std::vector<WidgetHandle> getEdgesOfFace(int f);
      int getOppositeFaceIndex(int f);
      std::vector<WidgetHandle> getWidgetsOnFace(int f);
      std::vector<WidgetHandle> getWidgetsOnOppositeFace(int f);
      std::vector<WidgetHandle> getFaceWidgetsParrallelToFace(int f);
      std::vector<WidgetHandle> getEdgesParrallelToFace(int f);

     private:
      const std::string TRANSLATE_COLOR_;
      const std::string ROTATE_COLOR_;
      const std::string SCALE_COLOR_;
      const std::string SCALE_AXIS_COLOR_;
      double scale_;
      std::vector<Core::Geometry::Vector> scaledEigvecs_;
      std::vector<Core::Geometry::Point> cornerPoints_;
      std::vector<Core::Geometry::Point> facePoints_;
      std::vector<WidgetHandle> corners_;
      std::vector<WidgetHandle> edges_;
      std::vector<WidgetHandle> faceDisks_;
      std::vector<WidgetHandle> faceSpheres_;

     public:
      const static int CORNER_COUNT_ = 8;
      const static int EDGE_COUNT_ = 12;
      const static int FACE_COUNT_ = 6;
      };

      using BoundingBoxWidgetHandle = SharedPointer<BoundingBoxWidget>;
    }
  }
}
#endif
