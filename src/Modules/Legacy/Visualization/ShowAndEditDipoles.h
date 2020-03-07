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


#ifndef MODULES_LEGACY_VISUALIZATION_SHOWANDEDITDIPOLES_H
#define MODULES_LEGACY_VISUALIZATION_SHOWANDEDITDIPOLES_H

#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Modules/Legacy/Visualization/share.h>

namespace SCIRun {

  namespace Core {
    namespace Algorithms {
      namespace Visualization {
        ALGORITHM_PARAMETER_DECL(FieldName);
        ALGORITHM_PARAMETER_DECL(WidgetScaleFactor);
        ALGORITHM_PARAMETER_DECL(Sizing);
        ALGORITHM_PARAMETER_DECL(ShowLastAsVector);
        ALGORITHM_PARAMETER_DECL(ShowLines);
        ALGORITHM_PARAMETER_DECL(Reset);
        ALGORITHM_PARAMETER_DECL(MoveDipolesTogether);
        ALGORITHM_PARAMETER_DECL(DipolePositions);
        ALGORITHM_PARAMETER_DECL(DipoleDirections);
        ALGORITHM_PARAMETER_DECL(DipoleScales);
        ALGORITHM_PARAMETER_DECL(DataSaved);
        ALGORITHM_PARAMETER_DECL(LargestSize);
      }}}

  namespace Modules {
    namespace Visualization {

      class ShowAndEditDipolesImpl;

      enum class SizingType
      {
        ORIGINAL,
        NORMALIZE_VECTOR_DATA,
        NORMALIZE_BY_LARGEST_VECTOR
      };

      class SCISHARE ShowAndEditDipoles : public SCIRun::Dataflow::Networks::GeometryGeneratingModule,
        public Has1InputPort<FieldPortTag>,
        public Has2OutputPorts<FieldPortTag, GeometryPortTag>
      {
      public:
        ShowAndEditDipoles();
        void execute() override;
        void setStateDefaults() override;

        INPUT_PORT(0, DipoleInputField, Field);
        OUTPUT_PORT(0, DipoleOutputField, Field);
        OUTPUT_PORT(1, DipoleWidget, GeometryObject);

        MODULE_TRAITS_AND_INFO(ModuleHasUI);

      private:
        SharedPointer<ShowAndEditDipolesImpl> impl_;
        void processWidgetFeedback(const Core::Datatypes::ModuleFeedback &var);
      };
    }
  }
};

#endif
