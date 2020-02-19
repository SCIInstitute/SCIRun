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


#ifndef MODULES_LEGACY_FIELDS_GenerateSinglePointProbeFromField_H__
#define MODULES_LEGACY_FIELDS_GenerateSinglePointProbeFromField_H__

#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Geometry.h>
#include <Modules/Legacy/Fields/share.h>

namespace SCIRun {

  namespace Core
  {
    namespace Algorithms
    {
      namespace Fields
      {
        ALGORITHM_PARAMETER_DECL(XLocation);
        ALGORITHM_PARAMETER_DECL(YLocation);
        ALGORITHM_PARAMETER_DECL(ZLocation);
        ALGORITHM_PARAMETER_DECL(MoveMethod);
        ALGORITHM_PARAMETER_DECL(FieldValue);
        ALGORITHM_PARAMETER_DECL(FieldNode);
        ALGORITHM_PARAMETER_DECL(FieldElem);
        ALGORITHM_PARAMETER_DECL(ProbeSize);
        ALGORITHM_PARAMETER_DECL(ProbeLabel);
        ALGORITHM_PARAMETER_DECL(ProbeColor);
        ALGORITHM_PARAMETER_DECL(SnapToNode);
        ALGORITHM_PARAMETER_DECL(SnapToElement);
      }
    }
  }

  namespace Modules {
    namespace Fields {

      class SCISHARE GenerateSinglePointProbeFromField : public Dataflow::Networks::GeometryGeneratingModule,
        public Has1InputPort<FieldPortTag>,
        public Has3OutputPorts<GeometryPortTag, FieldPortTag, ScalarPortTag>
      {
      public:
        GenerateSinglePointProbeFromField();

        virtual void execute() override;
        virtual void setStateDefaults() override;

        INPUT_PORT(0, InputField, Field);
        OUTPUT_PORT(0, GeneratedWidget, GeometryObject);
        OUTPUT_PORT(1, GeneratedPoint, Field);
        OUTPUT_PORT(2, ElementIndex, Int32);

        MODULE_TRAITS_AND_INFO(ModuleHasUI)
      private:
        boost::shared_ptr<class GenerateSinglePointProbeFromFieldImpl> impl_;
        Core::Geometry::Point currentLocation() const;
        void processWidgetFeedback(const Core::Datatypes::ModuleFeedback& var);
        void adjustPositionFromTransform(const Core::Geometry::Transform& transformMatrix);

        FieldHandle GenerateOutputField(boost::optional<FieldHandle> ifieldOption);
        index_type GenerateIndex();
        void setNearestNode(const Core::Geometry::Point& location);
        void setNearestElement(const Core::Geometry::Point& location);
      };
    }
  }
}

#endif
