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
/// @todo Documentation Modules/Visualization/ShowFieldGlyphs.h

#ifndef MODULES_VISUALIZATION_SHOW_FIELD_GLYPHS_H
#define MODULES_VISUALIZATION_SHOW_FIELD_GLYPHS_H

#include <Dataflow/Network/Module.h>
#include <Core/Thread/Interruptible.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Visualization {

      class SCISHARE ShowFieldGlyphs : public Dataflow::Networks::GeometryGeneratingModule,
        //public Has6InputPorts<FieldPortTag, ColorMapPortTag, FieldPortTag, ColorMapPortTag, FieldPortTag, ColorMapPortTag>,
        public Has2InputPorts<FieldPortTag, ColorMapPortTag>,
        public Has1OutputPort < GeometryPortTag >,
        public Core::Thread::Interruptible
      {
      public:
        ShowFieldGlyphs();
        virtual void execute() override;

        // Vector Tab
        static const Core::Algorithms::AlgorithmParameterName ShowVectors;
        static const Core::Algorithms::AlgorithmParameterName VectorsTransparency;
        static const Core::Algorithms::AlgorithmParameterName VectorsTransparencyValue;
        static const Core::Algorithms::AlgorithmParameterName VectorsScale;
        static const Core::Algorithms::AlgorithmParameterName VectorsResolution;
        static const Core::Algorithms::AlgorithmParameterName VectorsColoring;
        static const Core::Algorithms::AlgorithmParameterName VectorsDisplayType;

        // Scalar Tab
        static const Core::Algorithms::AlgorithmParameterName ShowScalars;
        static const Core::Algorithms::AlgorithmParameterName ScalarsTransparency;
        static const Core::Algorithms::AlgorithmParameterName ScalarsTransparencyValue;
        static const Core::Algorithms::AlgorithmParameterName ScalarsScale;
        static const Core::Algorithms::AlgorithmParameterName ScalarsResolution;
        static const Core::Algorithms::AlgorithmParameterName ScalarsColoring;
        static const Core::Algorithms::AlgorithmParameterName ScalarsDisplayType;

        // Tensor Tab
        static const Core::Algorithms::AlgorithmParameterName ShowTensors;
        static const Core::Algorithms::AlgorithmParameterName TensorsTransparency;
        static const Core::Algorithms::AlgorithmParameterName TensorsTransparencyValue;
        static const Core::Algorithms::AlgorithmParameterName TensorsScale;
        static const Core::Algorithms::AlgorithmParameterName TensorsResolution;
        static const Core::Algorithms::AlgorithmParameterName TensorsColoring;
        static const Core::Algorithms::AlgorithmParameterName TensorsDisplayType;

        // Mesh Color
        static const Core::Algorithms::AlgorithmParameterName DefaultMeshColor;

        // Tab Control
        static const Core::Algorithms::AlgorithmParameterName ShowVectorTab;
        static const Core::Algorithms::AlgorithmParameterName ShowScalarTab;
        static const Core::Algorithms::AlgorithmParameterName ShowTensorTab;
        static const Core::Algorithms::AlgorithmParameterName ShowSecondaryTab;
        static const Core::Algorithms::AlgorithmParameterName ShowTertiaryTab;

        INPUT_PORT(0, PrimaryData, LegacyField);
        INPUT_PORT(1, PrimaryColorMap, ColorMap);
        //INPUT_PORT(2, SecondaryData, LegacyField);
        //INPUT_PORT(3, SecondaryColorMap, ColorMap);
        //INPUT_PORT(4, TertiaryData, LegacyField);
        //INPUT_PORT(5, TertiaryColorMap, ColorMap);
        OUTPUT_PORT(0, SceneGraph, GeometryObject);

        static const Dataflow::Networks::ModuleLookupInfo staticInfo_;

        virtual void setStateDefaults() override;

      private:
        void configureInputs(
          FieldHandle pfield,
          boost::optional<FieldHandle> sfield,
          boost::optional<FieldHandle> tfield,
          boost::optional<Core::Datatypes::ColorMapHandle> pcolormap,
          boost::optional<Core::Datatypes::ColorMapHandle> scolormap,
          boost::optional<Core::Datatypes::ColorMapHandle> tcolormap);

        boost::shared_ptr<class GlyphBuilder> builder_;

      };
    } // Visualization
  } // Modules
} // SCIRun

#endif //MODULES_VISUALIZATION_SHOW_FIELD_GLYPHS_H