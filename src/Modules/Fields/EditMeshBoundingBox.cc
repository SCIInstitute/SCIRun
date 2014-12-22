/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Modules/Fields/EditMeshBoundingBox.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

const ModuleLookupInfo EditMeshBoundingBoxModule::staticInfo_("EditMeshBoundingBox", "ChangeMesh", "SCIRun");

EditMeshBoundingBoxModule::EditMeshBoundingBoxModule() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(Transformation_Widget);
  INITIALIZE_PORT(Transformation_Matrix);
}

void EditMeshBoundingBoxModule::setStateDefaults()
{
  //TODO
}

void EditMeshBoundingBoxModule::execute()
{
  //TODO
}

AlgorithmParameterName EditMeshBoundingBoxModule::InputCenterX("InputCenterX");
AlgorithmParameterName EditMeshBoundingBoxModule::InputCenterY("InputCenterY");
AlgorithmParameterName EditMeshBoundingBoxModule::InputCenterZ("InputCenterZ");
AlgorithmParameterName EditMeshBoundingBoxModule::InputSizeX("InputSizeX");
AlgorithmParameterName EditMeshBoundingBoxModule::InputSizeY("InputSizeY");
AlgorithmParameterName EditMeshBoundingBoxModule::InputSizeZ("InputSizeZ");
//Output Field Atributes
AlgorithmParameterName EditMeshBoundingBoxModule::UseOutputCenter("UseOutputCenter");
AlgorithmParameterName EditMeshBoundingBoxModule::UseOutputSize("UseOutputSize");
AlgorithmParameterName EditMeshBoundingBoxModule::OutputCenterX("OutputCenterX");
AlgorithmParameterName EditMeshBoundingBoxModule::OutputCenterY("OutputCenterY");
AlgorithmParameterName EditMeshBoundingBoxModule::OutputCenterZ("OutputCenterZ");
AlgorithmParameterName EditMeshBoundingBoxModule::OutputSizeX("OutputSizeX");
AlgorithmParameterName EditMeshBoundingBoxModule::OutputSizeY("OutputSizeY");
AlgorithmParameterName EditMeshBoundingBoxModule::OutputSizeZ("OutputSizeZ");
//Widget Scale/Mode
AlgorithmParameterName EditMeshBoundingBoxModule::DoubleScaleUp("DoubleScaleUp");
AlgorithmParameterName EditMeshBoundingBoxModule::ScaleUp("ScaleUp");
AlgorithmParameterName EditMeshBoundingBoxModule::ScaleDown("ScaleDown");
AlgorithmParameterName EditMeshBoundingBoxModule::DoubleScaleDown("DoubleScaleDown");
AlgorithmParameterName EditMeshBoundingBoxModule::NoTranslation("NoTranslation");
AlgorithmParameterName EditMeshBoundingBoxModule::XYZTranslation("XYZTranslation");
AlgorithmParameterName EditMeshBoundingBoxModule::RDITranslation("RDITranslation");
AlgorithmParameterName EditMeshBoundingBoxModule::RestrictX("RestrictX");
AlgorithmParameterName EditMeshBoundingBoxModule::RestrictY("RestrictY");
AlgorithmParameterName EditMeshBoundingBoxModule::RestrictZ("RestrictZ");
AlgorithmParameterName EditMeshBoundingBoxModule::RestrictR("RestrictR");
AlgorithmParameterName EditMeshBoundingBoxModule::RestrictD("RestrictD");
AlgorithmParameterName EditMeshBoundingBoxModule::RestrictI("RestrictI");