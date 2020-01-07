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

#ifndef Graphics_Widgets_BoundingBoxWidget_H
#define Graphics_Widgets_BoundingBoxWidget_H

#include <Core/Datatypes/Legacy/Field/FieldFwd.h>
#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Graphics/Widgets/Widget.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun {
  namespace Graphics {
    namespace Datatypes {
      enum BoundingBoxWidgetSection {
        BOX,
        CORNER_SCALE,
        FACE_ROTATE,
        FACE_SCALE,
        X_PLUS,
        Y_PLUS,
        Z_PLUS,
        X_MINUS,
        Y_MINUS,
        Z_MINUS };

      class SCISHARE BoundingBoxWidget : public CompositeWidget
      {
      public:
        BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator, const std::string& name,
                          double scale, const Core::Geometry::Point& center,
                          const std::vector<Core::Geometry::Vector>& eigvecs,
                          const std::vector<double>& eigvals, int widget_num);
        BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator, const std::string& name,
                          double scale, const BoxPosition& pos, const Core::Geometry::Point& center,
                          int widget_num, const Core::Geometry::BBox& bbox);
        BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator, const std::string& name,
                          double scale, const Core::Geometry::Transform& trans,
                          const Core::Geometry::Point& center, int widget_num);

      private:
        int RESOLUTION_ = 10;
        const int DIMENSIONS_ = 3;
        const int CORNERS_ = 8;
        const int FACES_ = 6;
        const float BOX_SCALE_ {1.0};
        const float ROT_SPHERE_SCALE_ {2.0};
        const float RESIZE_SPHERE_SCALE_ {1.5};
        const float DISK_WIDTH_ {3.0};
        const float DISK_RADIUS_ {1.0};
        const std::string DEFL_POINT_COL_ {Core::Datatypes::ColorRGB(0.54, 0.6, 1.0).toString()};
        const std::string DEFL_COL_ {Core::Datatypes::ColorRGB(0.5, 0.5, 0.5).toString()};
        const std::string RESIZE_COL_ {Core::Datatypes::ColorRGB(0.54, 1.0, 0.60).toString()};
        const std::string DISK_COL_ = RESIZE_COL_;
        std::vector<WidgetHandle> translateWidgets_;
        std::vector<WidgetHandle> rotateWidgets_;
        std::vector<WidgetHandle> scaleWidgets_;
        std::vector<std::vector<std::vector<WidgetHandle>>> scaleAxisWidgets_;
        std::vector<std::string> allIds_;
        std::vector<std::string> translateIds_;
        std::vector<std::vector<std::vector<std::string>>> translateIdsByFace_;
        std::vector<std::vector<std::string> > translateIdsBySide_;
        std::vector<std::vector<std::vector<std::string>>> rotateIdsByFace_;
        std::vector<std::vector<std::vector<std::string>>> scaleIdsByFace_;
        std::vector<std::string> rotateIds_;
        std::vector<std::string> scaleIds_;
        std::vector<std::vector<std::vector<std::string>>> scaleAxisIds_;
        std::string widgetName(size_t i, size_t id, size_t iter);

        int widgetsIndex_;
        double scale_;
        double smallestEigval_;
        Core::Geometry::BBox bbox_;
        Core::Geometry::Point center_;
        std::vector<double> eigvals_;
        std::vector<Core::Geometry::Vector> eigvecs_;
        std::vector<Core::Geometry::Vector> scaledEigvecs_;
        std::vector<Core::Geometry::Point> corners_;
        std::vector<Core::Geometry::Point> facesStart_;
        std::vector<Core::Geometry::Point> facesEnd_;

        void addBox(const Core::GeometryIDGenerator& idGenerator, int widgetNum);
        void createWidgets(const Core::GeometryIDGenerator& idGenerator, int widgetNum);
        void initWidgetCreation(const Core::GeometryIDGenerator& idGenerator, int widgetNum);
        void getEigenValuesAndEigenVectors();
        void getCorners();
        void getFacesStart();
        void getFacesEnd();
        void addCornerSpheres(const Core::GeometryIDGenerator& idGenerator, int widgetNum);
        void addFaceSphere(const Core::GeometryIDGenerator& idGenerator, int widgetNum);
        void addFaceCylinder(const Core::GeometryIDGenerator& idGenerator,
                             glm::mat4& scaleTrans, int widgetNum);
        void getTranslateIds();
        void getRotateIds();
        void getScaleIds();
        void getScaleAxisIds();
        void generateWidgetPoints();
        void assignIds();
        void assignMoveMaps();
        void assignTranslateMaps();
        void assignRotateMaps();
        void assignScaleMaps();
        void assignScaleAxisMaps();
        void assignScaleAxisUnidirectionalMaps();
      };

      using BoundingBoxWidgetHandle = SharedPointer<BoundingBoxWidget>;
    }
  }
}
#endif
