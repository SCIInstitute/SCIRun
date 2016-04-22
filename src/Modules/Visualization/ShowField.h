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
/// @todo Documentation Modules/Visualization/ShowField.h

#ifndef MODULES_VISUALIZATION_SHOW_FIELD_H
#define MODULES_VISUALIZATION_SHOW_FIELD_H

#include <Dataflow/Network/Module.h>
#include <Core/Thread/Interruptible.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {

  namespace Core
  {
    namespace Algorithms
    {
      namespace Visualization
      {
        ALGORITHM_PARAMETER_DECL(CylinderRadius);
      }
    }
  }

  namespace Modules {
    namespace Visualization {

      namespace detail
      {
        class GeometryBuilder;
      }

      class SCISHARE ShowFieldModule : public Dataflow::Networks::GeometryGeneratingModule,
        public Has2InputPorts<FieldPortTag, ColorMapPortTag>,
        public Has1OutputPort<GeometryPortTag>,
        public Core::Thread::Interruptible
      {
      public:
        ShowFieldModule();
        virtual void execute() override;

        static const Core::Algorithms::AlgorithmParameterName FieldName;
        static const Core::Algorithms::AlgorithmParameterName NodesAvailable;
        static const Core::Algorithms::AlgorithmParameterName EdgesAvailable;
        static const Core::Algorithms::AlgorithmParameterName FacesAvailable;
        static const Core::Algorithms::AlgorithmParameterName ShowNodes;
        static const Core::Algorithms::AlgorithmParameterName ShowEdges;
        static const Core::Algorithms::AlgorithmParameterName ShowFaces;
        static const Core::Algorithms::AlgorithmParameterName NodeTransparency;
        static const Core::Algorithms::AlgorithmParameterName EdgeTransparency;
        static const Core::Algorithms::AlgorithmParameterName FaceTransparency;
        static const Core::Algorithms::AlgorithmParameterName FaceInvertNormals;
        static const Core::Algorithms::AlgorithmParameterName NodeAsPoints;
        static const Core::Algorithms::AlgorithmParameterName NodeAsSpheres;
        static const Core::Algorithms::AlgorithmParameterName EdgesAsLines;
        static const Core::Algorithms::AlgorithmParameterName EdgesAsCylinders;
        static const Core::Algorithms::AlgorithmParameterName DefaultMeshColor;
        static const Core::Algorithms::AlgorithmParameterName FaceTransparencyValue;
        static const Core::Algorithms::AlgorithmParameterName EdgeTransparencyValue;
        static const Core::Algorithms::AlgorithmParameterName NodeTransparencyValue;
        static const Core::Algorithms::AlgorithmParameterName SphereScaleValue;
        static const Core::Algorithms::AlgorithmParameterName CylinderResolution;
        static const Core::Algorithms::AlgorithmParameterName SphereResolution;
        static const Core::Algorithms::AlgorithmParameterName CylinderRadius;
        static const Core::Algorithms::AlgorithmParameterName DefaultTextColor;
        static const Core::Algorithms::AlgorithmParameterName ShowText;
        static const Core::Algorithms::AlgorithmParameterName ShowDataValues;
        static const Core::Algorithms::AlgorithmParameterName ShowNodeIndices;
        static const Core::Algorithms::AlgorithmParameterName ShowEdgeIndices;
        static const Core::Algorithms::AlgorithmParameterName ShowFaceIndices;
        static const Core::Algorithms::AlgorithmParameterName ShowCellIndices;
        static const Core::Algorithms::AlgorithmParameterName CullBackfacingText;
        static const Core::Algorithms::AlgorithmParameterName TextAlwaysVisible;
        static const Core::Algorithms::AlgorithmParameterName RenderAsLocation;
        static const Core::Algorithms::AlgorithmParameterName TextSize;
        static const Core::Algorithms::AlgorithmParameterName TextPrecision;
        static const Core::Algorithms::AlgorithmParameterName TextColoring;
        static const Core::Algorithms::AlgorithmParameterName UseFaceNormals;


        INPUT_PORT(0, Field, LegacyField);
        INPUT_PORT(1, ColorMapObject, ColorMap);
        OUTPUT_PORT(0, SceneGraph, GeometryObject);

        static const Dataflow::Networks::ModuleLookupInfo staticInfo_;

        virtual void setStateDefaults() override;
      private:
        void updateAvailableRenderOptions(FieldHandle field);

        boost::shared_ptr<detail::GeometryBuilder> builder_;
      };



    } // Visualization
  } // Modules
} // SCIRun

#endif
