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

#ifndef MODULES_VISUALIZATION_SHOW_UNCERTAINTY_GLYPHS_H_
#define MODULES_VISUALIZATION_SHOW_UNCERTAINTY_GLYPHS_H_

#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {
namespace Modules {
namespace Visualization {
  class SCISHARE ShowUncertaintyGlyphs : public SCIRun::Dataflow::Networks::GeometryGeneratingModule,
                                         public Has2InputPorts<FieldPortTag, MatrixPortTag>,
                                         public Has1OutputPort<GeometryPortTag>
  {
  public:
    ShowUncertaintyGlyphs();
    virtual void execute();
    virtual void setStateDefaults();

    INPUT_PORT(0, InputField, Field);
    INPUT_PORT(1, InputMatrix, Matrix);
    OUTPUT_PORT(0, OutputGeom, GeometryObject);

    MODULE_TRAITS_AND_INFO(ModuleFlags::ModuleHasUIAndAlgorithm);

    static const Core::Algorithms::AlgorithmParameterName FieldName;
    static const Core::Algorithms::AlgorithmParameterName DefaultMeshColor;
    static const Core::Algorithms::AlgorithmParameterName ShowTensors;
    static const Core::Algorithms::AlgorithmParameterName TensorsTransparency;
    static const Core::Algorithms::AlgorithmParameterName TensorsUniformTransparencyValue;
    static const Core::Algorithms::AlgorithmParameterName SuperquadricEmphasis;
    static const Core::Algorithms::AlgorithmParameterName NormalizeTensors;
    static const Core::Algorithms::AlgorithmParameterName TensorsScale;
    static const Core::Algorithms::AlgorithmParameterName TensorsResolution;
  };
}}}

#endif
