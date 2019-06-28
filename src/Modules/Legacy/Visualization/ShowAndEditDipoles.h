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

#ifndef MODULES_LEGACY_VISUALIZATION_SHOWANDEDITDIPOLES_H
#define MODULES_LEGACY_VISUALIZATION_SHOWANDEDITDIPOLES_H

#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Core/Datatypes/Geometry.h>
#include <Graphics/Widgets/Widget.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Modules/Legacy/Visualization/share.h>

enum SizingType {
        ORIGINAL,
        NORMALIZE_VECTOR_DATA,
        NORMALIZE_BY_LARGEST_VECTOR
};

namespace SCIRun {
  namespace Modules {
    namespace Visualization {

      class SCISHARE ShowAndEditDipoles : public SCIRun::Dataflow::Networks::GeometryGeneratingModule,
        public Has1InputPort<FieldPortTag>,
        public Has2OutputPorts<FieldPortTag, GeometryPortTag>
      {
      public:
        ShowAndEditDipoles();
        virtual void execute() override;
        virtual void setStateDefaults() override;

        static const Core::Algorithms::AlgorithmParameterName FieldName;
        static const Core::Algorithms::AlgorithmParameterName WidgetScaleFactor;
        static const Core::Algorithms::AlgorithmParameterName Sizing;
        static const Core::Algorithms::AlgorithmParameterName ShowLastAsVector;
        static const Core::Algorithms::AlgorithmParameterName ShowLines;
        static const Core::Algorithms::AlgorithmParameterName Reset;
        static const Core::Algorithms::AlgorithmParameterName MoveDipolesTogether;
        static const Core::Algorithms::AlgorithmParameterName DipolePositions;
        static const Core::Algorithms::AlgorithmParameterName DipoleDirections;
        static const Core::Algorithms::AlgorithmParameterName DipoleScales;
        static const Core::Algorithms::AlgorithmParameterName DataSaved;
        static const Core::Algorithms::AlgorithmParameterName LargestSize;

        INPUT_PORT(0, DipoleInputField, Field);
        OUTPUT_PORT(0, DipoleOutputField, Field);
        OUTPUT_PORT(1, DipoleWidget, GeometryObject);

        MODULE_TRAITS_AND_INFO(ModuleHasUI);

      private:
        std::vector<Core::Geometry::Point> pos_;
        std::vector<Core::Geometry::Vector> direction_;
        std::vector<double> scale_;
        Core::Geometry::BBox last_bounds_;
        std::vector<std::vector<Graphics::Datatypes::WidgetHandle>* > pointWidgets_;
        std::vector<Graphics::Datatypes::GeometryHandle> geoms_;
        std::vector<Core::Geometry::Transform> previousTransforms_;

        bool firstRun_;
        bool getFromFile_;
        bool lastVectorShown_;
        SizingType previousSizing_;
        double sphereRadius_;
        double cylinderRadius_;
        double coneRadius_;
        double diskRadius_;
        double diskDistFromCenter_;
        double diskWidth_;
        int widgetIter_;
        double resolution_;
        double previousScaleFactor_;
        double zeroVectorRescale_;

        Core::Datatypes::ColorRGB deflPointCol_;
        Core::Datatypes::ColorRGB deflCol_;
        Core::Datatypes::ColorRGB greenCol_;
        Core::Datatypes::ColorRGB resizeCol_;
        Core::Datatypes::ColorRGB lineCol_;

        void loadData();
        void refreshGeometry();
        void toggleLastVectorShown();
        void generateGeomsList();
        void ReceiveInputPoints();
        void ReceiveInputDirections();
        void ReceiveInputScales();
        void ReceiveInputField();
        void GenerateOutputGeom();
        void makeScalesPositive();
        void resetData();
        std::string widgetName(int i, int id, int iter);
        void createDipoleWidget(Core::Geometry::BBox& bbox, Core::Geometry::Point& pos, Core::Geometry::Vector dir, double scale, int widget_num, bool show_as_vector);
        void calculatePointMove(Core::Geometry::Point& oldPos, Core::Geometry::Point& newPos);
        void processWidgetFeedback(const Core::Datatypes::ModuleFeedback& var);
        void adjustPositionFromTransform(const Core::Geometry::Transform& transformMatrix, int index, int id);
        Graphics::Datatypes::GeometryHandle addLines();
        FieldHandle makePointCloud();
        void loadFromParameters();
        void saveToParameters();
      };
    }
  }
};

#endif
