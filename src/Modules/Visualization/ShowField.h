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
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Algorithms/Visualization/RenderFieldState.h>
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

      class SCISHARE ShowFieldModule : public SCIRun::Dataflow::Networks::GeometryGeneratingModule,
        public Has2InputPorts<FieldPortTag, ColorMapPortTag>,
        public Has1OutputPort<GeometryPortTag>,
        public Core::Thread::Interruptible
      {
      public:
        ShowFieldModule();
        virtual void execute();

        static Core::Algorithms::AlgorithmParameterName ShowNodes;
        static Core::Algorithms::AlgorithmParameterName ShowEdges;
        static Core::Algorithms::AlgorithmParameterName ShowFaces;
        static Core::Algorithms::AlgorithmParameterName NodeTransparency;
        static Core::Algorithms::AlgorithmParameterName EdgeTransparency;
        static Core::Algorithms::AlgorithmParameterName FaceTransparency;
        static Core::Algorithms::AlgorithmParameterName FaceInvertNormals;
        static Core::Algorithms::AlgorithmParameterName NodeAsPoints;
        static Core::Algorithms::AlgorithmParameterName NodeAsSpheres;
        static Core::Algorithms::AlgorithmParameterName EdgesAsLines;
        static Core::Algorithms::AlgorithmParameterName EdgesAsCylinders;
        static Core::Algorithms::AlgorithmParameterName DefaultMeshColor;
        static Core::Algorithms::AlgorithmParameterName FaceTransparencyValue;
        static Core::Algorithms::AlgorithmParameterName EdgeTransparencyValue;
        static Core::Algorithms::AlgorithmParameterName NodeTransparencyValue;
        static Core::Algorithms::AlgorithmParameterName SphereScaleValue;
        static Core::Algorithms::AlgorithmParameterName CylinderResolution;
        static Core::Algorithms::AlgorithmParameterName SphereResolution;
        static Core::Algorithms::AlgorithmParameterName CylinderRadius;

        INPUT_PORT(0, Field, LegacyField);
        INPUT_PORT(1, ColorMapObject, ColorMap);
        OUTPUT_PORT(0, SceneGraph, GeometryObject);

        static Dataflow::Networks::ModuleLookupInfo staticInfo_;

        virtual void setStateDefaults();
      private:
        /// Constructs a geometry object (essentially a spire object) from the given
        /// field data.
        /// \param field    Field from which to construct geometry.
        /// \param state
        /// \param id       Ends up becoming the name of the spire object.
        Core::Datatypes::GeometryHandle buildGeometryObject(
          boost::shared_ptr<SCIRun::Field> field,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
          Dataflow::Networks::ModuleStateHandle state);

        /// Mesh construction. Any of the functions below can modify the renderState.
        /// This modified render state will be passed onto the renderer.
        /// @{
        void renderNodes(
          boost::shared_ptr<SCIRun::Field> field,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
          RenderState state, Core::Datatypes::GeometryHandle geom,
          const std::string& id);

        void renderFaces(
          boost::shared_ptr<SCIRun::Field> field,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
          RenderState state, Core::Datatypes::GeometryHandle geom,
          unsigned int approx_div,
          const std::string& id);

        void renderFacesLinear(
          boost::shared_ptr<SCIRun::Field> field,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
          RenderState state, Core::Datatypes::GeometryHandle geom,
          unsigned int approxDiv,
          const std::string& id);

        void addFaceGeom(
          const std::vector<Core::Geometry::Point>  &points,
          const std::vector<Core::Geometry::Vector> &normals,
          bool withNormals,
          uint32_t& iboBufferIndex,
          CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer,
          CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer,
          Core::Datatypes::GeometryObject::ColorScheme colorScheme,
          const std::vector<SCIRun::Core::Datatypes::ColorRGB> &face_colors,
          const RenderState& state);

        void renderEdges(
          boost::shared_ptr<SCIRun::Field> field,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
          RenderState state,
          Core::Datatypes::GeometryHandle geom,
          const std::string& id);
        /// @}

        /// State evaluation
        /// @{
        RenderState getNodeRenderState(
          Dataflow::Networks::ModuleStateHandle state,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap);

        RenderState getEdgeRenderState(
          Dataflow::Networks::ModuleStateHandle state,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap);

        RenderState getFaceRenderState(
          Dataflow::Networks::ModuleStateHandle state,
          boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap);
        /// @}

        float faceTransparencyValue_;
        float edgeTransparencyValue_;
        float nodeTransparencyValue_;

      };

    } // Visualization
  } // Modules
} // SCIRun

#endif
