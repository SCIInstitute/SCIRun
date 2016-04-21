/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <Dataflow/Network/Module.h>
#include <Modules/Fields/BoxWidgetTypes.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Modules/Fields/share.h>

namespace SCIRun {

  class BoxWidgetInterface;
  typedef boost::shared_ptr<BoxWidgetInterface> BoxWidgetPtr;

  namespace Modules {
    namespace Fields {

      class EditMeshBoundingBoxImpl;

      class SCISHARE EditMeshBoundingBox : public Dataflow::Networks::GeometryGeneratingModule,
        public Has1InputPort<FieldPortTag>,
        public Has3OutputPorts < FieldPortTag, GeometryPortTag, MatrixPortTag >
      {
      public:
        EditMeshBoundingBox();
        virtual void execute() override;
        virtual void setStateDefaults() override;

        static const Core::Algorithms::AlgorithmParameterName Resetting;
        //Input Field Attributes
        static const Core::Algorithms::AlgorithmParameterName InputCenterX;
        static const Core::Algorithms::AlgorithmParameterName InputCenterY;
        static const Core::Algorithms::AlgorithmParameterName InputCenterZ;
        static const Core::Algorithms::AlgorithmParameterName InputSizeX;
        static const Core::Algorithms::AlgorithmParameterName InputSizeY;
        static const Core::Algorithms::AlgorithmParameterName InputSizeZ;
        //Output Field Atributes
        static const Core::Algorithms::AlgorithmParameterName UseOutputCenter;
        static const Core::Algorithms::AlgorithmParameterName UseOutputSize;
        static const Core::Algorithms::AlgorithmParameterName OutputCenterX;
        static const Core::Algorithms::AlgorithmParameterName OutputCenterY;
        static const Core::Algorithms::AlgorithmParameterName OutputCenterZ;
        static const Core::Algorithms::AlgorithmParameterName OutputSizeX;
        static const Core::Algorithms::AlgorithmParameterName OutputSizeY;
        static const Core::Algorithms::AlgorithmParameterName OutputSizeZ;
        //Widget Scale/Mode
        static const Core::Algorithms::AlgorithmParameterName Scale;
        static const Core::Algorithms::AlgorithmParameterName NoTranslation;
        static const Core::Algorithms::AlgorithmParameterName XYZTranslation;
        static const Core::Algorithms::AlgorithmParameterName RDITranslation;
        static const Core::Algorithms::AlgorithmParameterName RestrictX;
        static const Core::Algorithms::AlgorithmParameterName RestrictY;
        static const Core::Algorithms::AlgorithmParameterName RestrictZ;
        static const Core::Algorithms::AlgorithmParameterName RestrictR;
        static const Core::Algorithms::AlgorithmParameterName RestrictD;
        static const Core::Algorithms::AlgorithmParameterName RestrictI;

        static const Core::Algorithms::AlgorithmParameterName BoxMode;
        static const Core::Algorithms::AlgorithmParameterName BoxRealScale;

        INPUT_PORT(0, InputField, LegacyField);
        OUTPUT_PORT(0, OutputField, LegacyField);
        OUTPUT_PORT(1, Transformation_Widget, GeometryObject);
        OUTPUT_PORT(2, Transformation_Matrix, Matrix);

        static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
      private:
        void executeImpl(FieldHandle f);
        void clear_vals();
        void update_input_attributes(FieldHandle);
        void build_widget(FieldHandle, bool reset);
        bool isBoxEmpty() const;
        void widget_moved(bool);
        void createBoxWidget();
        void setBoxRestrictions();
        Core::Datatypes::GeometryBaseHandle buildGeometryObject();
        void processWidgetFeedback(const Core::Datatypes::ModuleFeedback& var);
        SCIRun::Core::Geometry::BBox bbox_;

        BoxWidgetPtr box_;
        boost::shared_ptr<EditMeshBoundingBoxImpl> impl_;
      };
    }
  }
}

#endif
