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
        int mRESOLUTION = 10;
        const int mDIMENSIONS = 3;
        const int mEDGES = 12;
        const int mCORNERS = 8;
        const int mFACES = 6;
        const float mBOX_SCALE {1.0};
        const float mROT_SPHERE_SCALE {2.0};
        const float mRESIZE_SPHERE_SCALE {1.5};
        const float mDISK_WIDTH {3.0};
        const float mDISK_RADIUS {1.0};
        const std::string mDEFL_POINT_COL {Core::Datatypes::ColorRGB(0.54, 0.6, 1.0).toString()};
        const std::string mDEFL_COL {Core::Datatypes::ColorRGB(0.5, 0.5, 0.5).toString()};
        const std::string mRESIZE_COL {Core::Datatypes::ColorRGB(0.54, 1.0, 0.60).toString()};
        const std::string mDISK_COL = mRESIZE_COL;
        std::vector<WidgetHandle> mTranslateWidgets;
        std::vector<WidgetHandle> mRotateWidgets;
        std::vector<WidgetHandle> mScaleWidgets;
        std::vector<std::vector<std::vector<WidgetHandle>>> mScaleAxisWidgets;
        std::vector<std::string> mAllIds;
        std::vector<std::string> mTranslateIds;
        std::vector<std::vector<std::vector<std::string>>> mTranslateIdsByFace;
        std::vector<std::vector<std::string> > mTranslateIdsBySide;
        std::vector<std::vector<std::vector<std::string>>> mRotateIdsByFace;
        std::vector<std::vector<std::vector<std::string>>> mScaleIdsByFace;
        std::vector<std::string> mRotateIds;
        std::vector<std::string> mScaleIds;
        std::vector<std::vector<std::vector<std::string>>> mScaleAxisIds;
        std::string widgetName(size_t i, size_t id, size_t iter);

        int mWidgetsIndex;
        double mScale;
        double mSmallestEigval;
        Core::Geometry::BBox mBbox;
        Core::Geometry::Point mCenter;
        std::vector<double> mEigvals;
        std::vector<Core::Geometry::Vector> mEigvecs;
        std::vector<Core::Geometry::Vector> mScaledEigvecs;
        std::vector<Core::Geometry::Point> mCorners;
        std::vector<Core::Geometry::Point> mFacesStart;
        std::vector<Core::Geometry::Point> mFacesEnd;

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
