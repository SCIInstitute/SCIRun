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


/// @todo Documentation Modules/Visualization/ShowField.h

#ifndef MODULES_VISUALIZATION_SHOW_FIELD_H
#define MODULES_VISUALIZATION_SHOW_FIELD_H

#include <Dataflow/Network/GeometryGeneratingModule.h>
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
        ALGORITHM_PARAMETER_DECL(FieldName);
        ALGORITHM_PARAMETER_DECL(NodesAvailable);
        ALGORITHM_PARAMETER_DECL(EdgesAvailable);
        ALGORITHM_PARAMETER_DECL(FacesAvailable);
        ALGORITHM_PARAMETER_DECL(ShowNodes);
        ALGORITHM_PARAMETER_DECL(ShowEdges);
        ALGORITHM_PARAMETER_DECL(ShowFaces);
        ALGORITHM_PARAMETER_DECL(NodeTransparency);
        ALGORITHM_PARAMETER_DECL(EdgeTransparency);
        ALGORITHM_PARAMETER_DECL(FaceTransparency);
        ALGORITHM_PARAMETER_DECL(FaceInvertNormals);
        ALGORITHM_PARAMETER_DECL(NodeAsPoints);
        ALGORITHM_PARAMETER_DECL(NodeAsSpheres);
        ALGORITHM_PARAMETER_DECL(EdgesAsLines);
        ALGORITHM_PARAMETER_DECL(EdgesAsCylinders);
        ALGORITHM_PARAMETER_DECL(DefaultMeshColor);
        ALGORITHM_PARAMETER_DECL(FaceTransparencyValue);
        ALGORITHM_PARAMETER_DECL(EdgeTransparencyValue);
        ALGORITHM_PARAMETER_DECL(NodeTransparencyValue);
        ALGORITHM_PARAMETER_DECL(FacesColoring);
        ALGORITHM_PARAMETER_DECL(NodesColoring);
        ALGORITHM_PARAMETER_DECL(EdgesColoring);
        ALGORITHM_PARAMETER_DECL(SphereScaleValue);
        ALGORITHM_PARAMETER_DECL(CylinderResolution);
        ALGORITHM_PARAMETER_DECL(SphereResolution);
        ALGORITHM_PARAMETER_DECL(DefaultTextColor);
        ALGORITHM_PARAMETER_DECL(ShowText);
        ALGORITHM_PARAMETER_DECL(ShowDataValues);
        ALGORITHM_PARAMETER_DECL(ShowNodeIndices);
        ALGORITHM_PARAMETER_DECL(ShowEdgeIndices);
        ALGORITHM_PARAMETER_DECL(ShowFaceIndices);
        ALGORITHM_PARAMETER_DECL(ShowCellIndices);
        ALGORITHM_PARAMETER_DECL(CullBackfacingText);
        ALGORITHM_PARAMETER_DECL(TextAlwaysVisible);
        ALGORITHM_PARAMETER_DECL(RenderAsLocation);
        ALGORITHM_PARAMETER_DECL(TextSize);
        ALGORITHM_PARAMETER_DECL(TextPrecision);
        ALGORITHM_PARAMETER_DECL(TextColoring);
        ALGORITHM_PARAMETER_DECL(UseFaceNormals);
      }
    }
  }

  namespace Modules {
    namespace Visualization {

      namespace detail
      {
        class GeometryBuilder;
      }

      class SCISHARE ShowField : public Dataflow::Networks::GeometryGeneratingModule,
        public Has2InputPorts<FieldPortTag, ColorMapPortTag>,
        public Has1OutputPort<GeometryPortTag>,
        public Core::Thread::Interruptible
      {
      public:
        ShowField();
        virtual void execute() override;

        INPUT_PORT(0, Field, Field);
        INPUT_PORT(1, ColorMapObject, ColorMap);
        OUTPUT_PORT(0, SceneGraph, GeometryObject);
        //OUTPUT_PORT(1, OspraySceneGraph, OsprayGeometryObject); //TODO

        MODULE_TRAITS_AND_INFO(ModuleHasUI)

        virtual void setStateDefaults() override;
      private:
        void updateAvailableRenderOptions(FieldHandle field);
        void processMeshComponentSelection(const Core::Datatypes::ModuleFeedback& var);

        boost::shared_ptr<detail::GeometryBuilder> builder_;
      };

    } // Visualization
  } // Modules
} // SCIRun

#endif
