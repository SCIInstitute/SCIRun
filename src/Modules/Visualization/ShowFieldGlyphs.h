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
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Thread/Interruptible.h>
#include <Core/Algorithms/Visualization/RenderFieldState.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Visualization {

      class SCISHARE ShowFieldGlyphs : public SCIRun::Dataflow::Networks::GeometryGeneratingModule,
        //public Has6InputPorts<FieldPortTag, ColorMapPortTag, FieldPortTag, ColorMapPortTag, FieldPortTag, ColorMapPortTag>,
        public Has2InputPorts<FieldPortTag, ColorMapPortTag>,
        public Has1OutputPort < GeometryPortTag >,
        public Core::Thread::Interruptible
      {
      public:
        ShowFieldGlyphs();
        virtual void execute();

        // Vector Tab
        static Core::Algorithms::AlgorithmParameterName ShowVectors;
        static Core::Algorithms::AlgorithmParameterName VectorsTransparency;
        static Core::Algorithms::AlgorithmParameterName VectorsTransparencyValue;
        static Core::Algorithms::AlgorithmParameterName VectorsScale;
        static Core::Algorithms::AlgorithmParameterName VectorsResolution;
        static Core::Algorithms::AlgorithmParameterName VectorsColoring;
        static Core::Algorithms::AlgorithmParameterName VectorsDisplayType;

        // Scalar Tab
        static Core::Algorithms::AlgorithmParameterName ShowScalars;
        static Core::Algorithms::AlgorithmParameterName ScalarsTransparency;
        static Core::Algorithms::AlgorithmParameterName ScalarsTransparencyValue;
        static Core::Algorithms::AlgorithmParameterName ScalarsScale;
        static Core::Algorithms::AlgorithmParameterName ScalarsResolution;
        static Core::Algorithms::AlgorithmParameterName ScalarsColoring;
        static Core::Algorithms::AlgorithmParameterName ScalarsDisplayType;

        // Tensor Tab
        static Core::Algorithms::AlgorithmParameterName ShowTensors;
        static Core::Algorithms::AlgorithmParameterName TensorsTransparency;
        static Core::Algorithms::AlgorithmParameterName TensorsTransparencyValue;
        static Core::Algorithms::AlgorithmParameterName TensorsScale;
        static Core::Algorithms::AlgorithmParameterName TensorsResolution;
        static Core::Algorithms::AlgorithmParameterName TensorsColoring;
        static Core::Algorithms::AlgorithmParameterName TensorsDisplayType;

        // Mesh Color
        static Core::Algorithms::AlgorithmParameterName DefaultMeshColor;

        // Tab Control
        static Core::Algorithms::AlgorithmParameterName ShowVectorTab;
        static Core::Algorithms::AlgorithmParameterName ShowScalarTab;
        static Core::Algorithms::AlgorithmParameterName ShowTensorTab;
        static Core::Algorithms::AlgorithmParameterName ShowSecondaryTab;
        static Core::Algorithms::AlgorithmParameterName ShowTertiaryTab;

        INPUT_PORT(0, PrimaryData, LegacyField);
        INPUT_PORT(1, PrimaryColorMap, ColorMap);
        //INPUT_PORT(2, SecondaryData, LegacyField);
        //INPUT_PORT(3, SecondaryColorMap, ColorMap);
        //INPUT_PORT(4, TertiaryData, LegacyField);
        //INPUT_PORT(5, TertiaryColorMap, ColorMap);
        OUTPUT_PORT(0, SceneGraph, GeometryObject);

        static Dataflow::Networks::ModuleLookupInfo staticInfo_;

        virtual void setStateDefaults();

      private:
        void configureInputs(
          boost::shared_ptr<SCIRun::Field> pfield,
          boost::optional<boost::shared_ptr<SCIRun::Field>> sfield,
          boost::optional<boost::shared_ptr<SCIRun::Field>> tfield,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> pcolormap,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> scolormap,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> tcolormap);

        /// Constructs a geometry object (essentially a spire object) from the given
        /// field data.
        /// \param field    Field from which to construct geometry.
        /// \param state
        /// \param id       Ends up becoming the name of the spire object.
        Core::Datatypes::GeometryHandle buildGeometryObject(
          boost::shared_ptr<SCIRun::Field> field,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
          Dataflow::Networks::ModuleStateHandle state);

        void renderVectors(
          boost::shared_ptr<SCIRun::Field> field,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
          RenderState state,
          Core::Datatypes::GeometryHandle geom,
          const std::string& id);

        void renderScalars(
          boost::shared_ptr<SCIRun::Field> field,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
          RenderState state,
          Core::Datatypes::GeometryHandle geom,
          const std::string& id);

        void renderTensors(
          boost::shared_ptr<SCIRun::Field> field,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
          RenderState state,
          Core::Datatypes::GeometryHandle geom,
          const std::string& id);

        RenderState getVectorsRenderState(
          Dataflow::Networks::ModuleStateHandle state,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap);   

        RenderState getScalarsRenderState(
          Dataflow::Networks::ModuleStateHandle state,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap);

        RenderState getTensorsRenderState(
          Dataflow::Networks::ModuleStateHandle state,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap);
      };
    } // Visualization
  } // Modules
} // SCIRun

#endif //MODULES_VISUALIZATION_SHOW_FIELD_GLYPHS_H