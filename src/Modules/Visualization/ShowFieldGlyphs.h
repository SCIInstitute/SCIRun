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


/// @todo Documentation Modules/Visualization/ShowFieldGlyphs.h

#ifndef MODULES_VISUALIZATION_SHOW_FIELD_GLYPHS_H
#define MODULES_VISUALIZATION_SHOW_FIELD_GLYPHS_H

#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Core/Thread/Interruptible.h>
#include <Core/Algorithms/Visualization/RenderFieldState.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Visualization {

      class SCISHARE ShowFieldGlyphs : public Dataflow::Networks::GeometryGeneratingModule,
        public Has6InputPorts<FieldPortTag, ColorMapPortTag, FieldPortTag, ColorMapPortTag, FieldPortTag, ColorMapPortTag>,
        public Has1OutputPort < GeometryPortTag >,
        public Core::Thread::Interruptible
      {
      public:
        ShowFieldGlyphs();
        virtual void execute() override;

        static const Core::Algorithms::AlgorithmParameterName FieldName;
        // Mesh Color
        static const Core::Algorithms::AlgorithmParameterName DefaultMeshColor;
        // Vector Controls
        static const Core::Algorithms::AlgorithmParameterName ShowVectorTab;
        static const Core::Algorithms::AlgorithmParameterName ShowVectors;
        static const Core::Algorithms::AlgorithmParameterName VectorsDisplayType;
        static const Core::Algorithms::AlgorithmParameterName VectorsColoring;
        static const Core::Algorithms::AlgorithmParameterName VectorsColoringDataInput;
        static const Core::Algorithms::AlgorithmParameterName VectorsTransparency;
        static const Core::Algorithms::AlgorithmParameterName VectorsUniformTransparencyValue;
        //        static const Core::Algorithms::AlgorithmParameterName VectorsTransparencyDataInput;
        static const Core::Algorithms::AlgorithmParameterName NormalizeVectors;
        static const Core::Algorithms::AlgorithmParameterName VectorsScale;
        static const Core::Algorithms::AlgorithmParameterName RenderVectorsBelowThreshold;
        static const Core::Algorithms::AlgorithmParameterName VectorsThreshold;
        static const Core::Algorithms::AlgorithmParameterName SecondaryVectorParameterScalingType;
        static const Core::Algorithms::AlgorithmParameterName SecondaryVectorParameterDataInput;
        static const Core::Algorithms::AlgorithmParameterName SecondaryVectorParameterScale;
        static const Core::Algorithms::AlgorithmParameterName ArrowHeadRatio;
        static const Core::Algorithms::AlgorithmParameterName RenderBidirectionaly;
        static const Core::Algorithms::AlgorithmParameterName RenderBases;
        static const Core::Algorithms::AlgorithmParameterName VectorsResolution;
        // Scalar Controls
        static const Core::Algorithms::AlgorithmParameterName ShowScalarTab;
        static const Core::Algorithms::AlgorithmParameterName ShowScalars;
        static const Core::Algorithms::AlgorithmParameterName ScalarsDisplayType;
        static const Core::Algorithms::AlgorithmParameterName ScalarsColoring;
        static const Core::Algorithms::AlgorithmParameterName ScalarsColoringDataInput;
        static const Core::Algorithms::AlgorithmParameterName ScalarsTransparency;
        static const Core::Algorithms::AlgorithmParameterName ScalarsUniformTransparencyValue;
        //        static const Core::Algorithms::AlgorithmParameterName ScalarsTransparencyDataInput;
        static const Core::Algorithms::AlgorithmParameterName ScalarsScale;
        static const Core::Algorithms::AlgorithmParameterName ScalarsThreshold;
        static const Core::Algorithms::AlgorithmParameterName ScalarsResolution;
        // Tensor Controls
        static const Core::Algorithms::AlgorithmParameterName ShowTensorTab;
        static const Core::Algorithms::AlgorithmParameterName ShowTensors;
        static const Core::Algorithms::AlgorithmParameterName TensorsDisplayType;
        static const Core::Algorithms::AlgorithmParameterName TensorsColoring;
        static const Core::Algorithms::AlgorithmParameterName TensorsColoringDataInput;
        static const Core::Algorithms::AlgorithmParameterName TensorsTransparency;
        static const Core::Algorithms::AlgorithmParameterName TensorsUniformTransparencyValue;
        static const Core::Algorithms::AlgorithmParameterName SuperquadricEmphasis;
        //        static const Core::Algorithms::AlgorithmParameterName TensorsTransparencyDataInput;
        static const Core::Algorithms::AlgorithmParameterName NormalizeTensors;
        static const Core::Algorithms::AlgorithmParameterName TensorsScale;
        static const Core::Algorithms::AlgorithmParameterName RenderTensorsBelowThreshold;
        static const Core::Algorithms::AlgorithmParameterName TensorsThreshold;
        static const Core::Algorithms::AlgorithmParameterName TensorsResolution;

        INPUT_PORT(0, PrimaryData, Field);
        INPUT_PORT(1, PrimaryColorMap, ColorMap);
        INPUT_PORT(2, SecondaryData, Field);
        INPUT_PORT(3, SecondaryColorMap, ColorMap);
        INPUT_PORT(4, TertiaryData, Field);
        INPUT_PORT(5, TertiaryColorMap, ColorMap);
        OUTPUT_PORT(0, SceneGraph, GeometryObject);

        virtual void setStateDefaults() override;

        MODULE_TRAITS_AND_INFO(ModuleHasUI)

      private:
        void configureInputs(
          FieldHandle pfield,
          boost::optional<FieldHandle> sfield,
          boost::optional<FieldHandle> tfield,
          boost::optional<Core::Datatypes::ColorMapHandle> pcolormap,
          boost::optional<Core::Datatypes::ColorMapHandle> scolormap,
          boost::optional<Core::Datatypes::ColorMapHandle> tcolormap);
        RenderState::InputPort getInput(const std::string &port_name);
        void setSuperquadricEmphasis(int emphasis);

        boost::shared_ptr<class GlyphBuilder> builder_;

      };
    } // Visualization
  } // Modules
} // SCIRun

#endif //MODULES_VISUALIZATION_SHOW_FIELD_GLYPHS_H
