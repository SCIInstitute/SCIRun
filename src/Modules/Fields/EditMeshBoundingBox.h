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

#ifndef MODULES_FIELDS_EDITMESHBOUNDINGBOX_H
#define MODULES_FIELDS_EDITMESHBOUNDINGBOX_H

#include <Core/Datatypes/Geometry.h>
#include <Core/GeometryPrimitives/OrientedBBox.h>
#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/Widget.h>
#include <Graphics/Widgets/BoundingBoxWidget.h>
#include <Core/Algorithms/Base/AlgorithmMacros.h>
#include <Modules/Fields/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {
      namespace Fields {
        ALGORITHM_PARAMETER_DECL(ScaleTransformMatrix);
        ALGORITHM_PARAMETER_DECL(RotationTransformMatrix);
        ALGORITHM_PARAMETER_DECL(TranslationPoint);
        ALGORITHM_PARAMETER_DECL(InverseFieldTransformMatrix);
        ALGORITHM_PARAMETER_DECL(ResetToInput);
        ALGORITHM_PARAMETER_DECL(ResetSize);
        ALGORITHM_PARAMETER_DECL(ResetCenter);
        ALGORITHM_PARAMETER_DECL(DataSaved);
        ALGORITHM_PARAMETER_DECL(InputCenterX);
        ALGORITHM_PARAMETER_DECL(InputCenterY);
        ALGORITHM_PARAMETER_DECL(InputCenterZ);
        ALGORITHM_PARAMETER_DECL(InputSizeX);
        ALGORITHM_PARAMETER_DECL(InputSizeY);
        ALGORITHM_PARAMETER_DECL(InputSizeZ);
        ALGORITHM_PARAMETER_DECL(SetOutputCenter);
        ALGORITHM_PARAMETER_DECL(SetOutputSize);
        ALGORITHM_PARAMETER_DECL(OutputCenterX);
        ALGORITHM_PARAMETER_DECL(OutputCenterY);
        ALGORITHM_PARAMETER_DECL(OutputCenterZ);
        ALGORITHM_PARAMETER_DECL(OutputSizeX);
        ALGORITHM_PARAMETER_DECL(OutputSizeY);
        ALGORITHM_PARAMETER_DECL(OutputSizeZ);
        ALGORITHM_PARAMETER_DECL(Scale);
        ALGORITHM_PARAMETER_DECL(OldScale);
        ALGORITHM_PARAMETER_DECL(ScaleChanged);
        ALGORITHM_PARAMETER_DECL(NoTranslation);
        ALGORITHM_PARAMETER_DECL(XYZTranslation);
        ALGORITHM_PARAMETER_DECL(RDITranslation);
        ALGORITHM_PARAMETER_DECL(RestrictX);
        ALGORITHM_PARAMETER_DECL(RestrictY);
        ALGORITHM_PARAMETER_DECL(RestrictZ);
        ALGORITHM_PARAMETER_DECL(RestrictR);
        ALGORITHM_PARAMETER_DECL(RestrictD);
        ALGORITHM_PARAMETER_DECL(RestrictI);
        ALGORITHM_PARAMETER_DECL(BoxMode);
        ALGORITHM_PARAMETER_DECL(BoxRealScale);
      }
    }
  }

  namespace Modules {
    namespace Fields {

      class EditMeshBoundingBoxImpl;

      class SCISHARE EditMeshBoundingBox : public Dataflow::Networks::GeometryGeneratingModule,
        public Has1InputPort<FieldPortTag>,
        public Has3OutputPorts < FieldPortTag, GeometryPortTag, MatrixPortTag >
      {
      public:
        static const int mDIMENSIONS = 3;
        EditMeshBoundingBox();
        void execute() override;
        void setStateDefaults() override;

        INPUT_PORT(0, InputField, Field);
        OUTPUT_PORT(0, OutputField, Field);
        OUTPUT_PORT(1, Transformation_Widget, GeometryObject);
        OUTPUT_PORT(2, Transformation_Matrix, Matrix);

        MODULE_TRAITS_AND_INFO(ModuleFlags::ModuleHasUI)

      private:
        const double HALF_SCALE_ = 0.5;

        void clearVals();
        void computeWidgetBox(const Core::Geometry::BBox& box);
        void buildGeometryObject();
        void updateState(FieldHandle field);
        void sendOutputPorts();
        void resetToInputField();
        void changeAxesOrientation(FieldHandle field);
        void setOutputCenter();
        void resetOutputCenter();
        void setOutputSize();
        void resetOutputSize();
        void processWidgetFeedback(const Core::Datatypes::ModuleFeedback& var);
        void adjustGeometryFromTransform(const Core::Geometry::Transform& feedbackTrans,
                                         const Core::Datatypes::WidgetMovement& movementType);
        void generateGeomsList();
        void saveToParameters();
        void loadFromParameters();
        std::string convertForLabel(double coord);
        void updateInputFieldAttributes();
        void setOutputParameters();

        Core::Geometry::BBox bbox_;
        Core::Geometry::Point pos_;
        std::vector<Core::Geometry::Vector> eigvecs_;
        Core::Geometry::Vector size_;
        FieldHandle outputField_;
        int widgetNum_{0};
        int resolution_ = 20;

        std::vector<Graphics::Datatypes::GeometryHandle> geoms_;
        Core::Geometry::Point ogPos_;
        Core::Geometry::Vector ogSize_;
        Core::Geometry::Transform inputFieldInverse_;
        Core::Geometry::Transform widgetScale_;
        Core::Geometry::Transform widgetRotation_;
        Core::Geometry::Point widgetTranslation_;
        bool widgetMoved_;
        bool widgetAxesRotated_;
        bool firstRun_;
      };
    }
  }
}

#endif
