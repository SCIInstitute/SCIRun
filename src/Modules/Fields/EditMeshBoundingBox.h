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

#ifndef MODULES_FIELDS_EDITMESHBOUNDINGBOX_H
#define MODULES_FIELDS_EDITMESHBOUNDINGBOX_H

#include <Dataflow/Network/Module.h>
#include <Modules/Fields/share.h>

namespace SCIRun {
namespace Modules {
namespace Fields {
  
  class SCISHARE EditMeshBoundingBoxModule : public SCIRun::Dataflow::Networks::Module,
    public Has1InputPort<FieldPortTag>,
    public Has3OutputPorts<FieldPortTag, GeometryPortTag, MatrixPortTag>
  {
  public:
    EditMeshBoundingBoxModule();
    virtual void execute();
    virtual void setStateDefaults();

	//Input Field Attributes
	static Core::Algorithms::AlgorithmParameterName InputCenterX;
	static Core::Algorithms::AlgorithmParameterName InputCenterY;
	static Core::Algorithms::AlgorithmParameterName InputCenterZ;
	static Core::Algorithms::AlgorithmParameterName InputSizeX;
	static Core::Algorithms::AlgorithmParameterName InputSizeY;
	static Core::Algorithms::AlgorithmParameterName InputSizeZ;
	//Output Field Atributes
	static Core::Algorithms::AlgorithmParameterName UseOutputCenter;
	static Core::Algorithms::AlgorithmParameterName UseOutputSize;
	static Core::Algorithms::AlgorithmParameterName OutputCenterX;
	static Core::Algorithms::AlgorithmParameterName OutputCenterY;
	static Core::Algorithms::AlgorithmParameterName OutputCenterZ;
	static Core::Algorithms::AlgorithmParameterName OutputSizeX;
	static Core::Algorithms::AlgorithmParameterName OutputSizeY;
	static Core::Algorithms::AlgorithmParameterName OutputSizeZ;
	//Widget Scale/Mode
	static Core::Algorithms::AlgorithmParameterName DoubleScaleUp;
	static Core::Algorithms::AlgorithmParameterName ScaleUp;
	static Core::Algorithms::AlgorithmParameterName ScaleDown;
	static Core::Algorithms::AlgorithmParameterName DoubleScaleDown;
	static Core::Algorithms::AlgorithmParameterName NoTranslation;
	static Core::Algorithms::AlgorithmParameterName XYZTranslation;
	static Core::Algorithms::AlgorithmParameterName RDITranslation;
	static Core::Algorithms::AlgorithmParameterName RestrictX;
	static Core::Algorithms::AlgorithmParameterName RestrictY;
	static Core::Algorithms::AlgorithmParameterName RestrictZ;
	static Core::Algorithms::AlgorithmParameterName RestrictR;
	static Core::Algorithms::AlgorithmParameterName RestrictD;
	static Core::Algorithms::AlgorithmParameterName RestrictI;

    INPUT_PORT(0, InputField, LegacyField);
    OUTPUT_PORT(0, OutputField, LegacyField);
    OUTPUT_PORT(1, Transformation_Widget, GeometryObject);
    OUTPUT_PORT(2, Transformation_Matrix, Matrix);	
	
	static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
  };
}}}

#endif