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

#ifndef MODULES_MATH_CREATEGEOMETRICTRANSFORMMODULE_H
#define MODULES_MATH_CREATEGEOMETRICTRANSFORMMODULE_H

#include <Dataflow/Network/Module.h>
#include <Modules/Math/share.h>

namespace SCIRun {
namespace Modules {
namespace Math {
  
  class SCISHARE CreateGeometricTransformModule : public SCIRun::Dataflow::Networks::Module,
    public Has1InputPort<MatrixPortTag>,
    public Has2OutputPorts<MatrixPortTag, GeometryPortTag>
  {
  public:
    CreateGeometricTransformModule();
    virtual void execute() override;

    static const Core::Algorithms::AlgorithmParameterName TransformType;
    static const Core::Algorithms::AlgorithmParameterName MultiplyRadioButton;
    static const Core::Algorithms::AlgorithmParameterName TranslateVectorX;
    static const Core::Algorithms::AlgorithmParameterName TranslateVectorY;
    static const Core::Algorithms::AlgorithmParameterName TranslateVectorZ;
    static const Core::Algorithms::AlgorithmParameterName ScalePointX;
    static const Core::Algorithms::AlgorithmParameterName ScalePointY;
    static const Core::Algorithms::AlgorithmParameterName ScalePointZ;
    static const Core::Algorithms::AlgorithmParameterName LogScale;
    static const Core::Algorithms::AlgorithmParameterName LogScaleX;
    static const Core::Algorithms::AlgorithmParameterName LogScaleY;
    static const Core::Algorithms::AlgorithmParameterName LogScaleZ;
    static const Core::Algorithms::AlgorithmParameterName RotatePointX;
    static const Core::Algorithms::AlgorithmParameterName RotatePointY;
    static const Core::Algorithms::AlgorithmParameterName RotatePointZ;
    static const Core::Algorithms::AlgorithmParameterName RotateAxisX;
    static const Core::Algorithms::AlgorithmParameterName RotateAxisY;
    static const Core::Algorithms::AlgorithmParameterName RotateAxisZ;
    static const Core::Algorithms::AlgorithmParameterName RotateTheta;
    static const Core::Algorithms::AlgorithmParameterName ShearVectorX;
    static const Core::Algorithms::AlgorithmParameterName ShearVectorY;
    static const Core::Algorithms::AlgorithmParameterName ShearVectorZ;
    static const Core::Algorithms::AlgorithmParameterName ShearPlaneA;
    static const Core::Algorithms::AlgorithmParameterName ShearPlaneB;
    static const Core::Algorithms::AlgorithmParameterName ShearPlaneC;
    static const Core::Algorithms::AlgorithmParameterName ShearPlaneD;
    static const Core::Algorithms::AlgorithmParameterName FieldMapX;
    static const Core::Algorithms::AlgorithmParameterName FieldMapY;
    static const Core::Algorithms::AlgorithmParameterName FieldMapZ;
    static const Core::Algorithms::AlgorithmParameterName UniformScale;
    static const Core::Algorithms::AlgorithmParameterName ResizeSeparably;
    static const Core::Algorithms::AlgorithmParameterName IgnoreChanges;

    virtual void setStateDefaults();
    static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
    
    INPUT_PORT(0, InputMatrix, Matrix);
    OUTPUT_PORT(0, OutputMatrix, Matrix);
    OUTPUT_PORT(1, Widget, GeometryObject);
  };
}}}

#endif
