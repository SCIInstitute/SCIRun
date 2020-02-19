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


#ifndef MODULES_MATH_CREATEGEOMETRICTRANSFORM_H
#define MODULES_MATH_CREATEGEOMETRICTRANSFORM_H

#include <Dataflow/Network/Module.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Modules/Math/share.h>

namespace SCIRun {

  namespace Core {
    namespace Algorithms {
      namespace Math {
        ALGORITHM_PARAMETER_DECL(TransformType);
        ALGORITHM_PARAMETER_DECL(MultiplyRadioButton);
        ALGORITHM_PARAMETER_DECL(TranslateVectorX);
        ALGORITHM_PARAMETER_DECL(TranslateVectorY);
        ALGORITHM_PARAMETER_DECL(TranslateVectorZ);
        ALGORITHM_PARAMETER_DECL(ScalePointX);
        ALGORITHM_PARAMETER_DECL(ScalePointY);
        ALGORITHM_PARAMETER_DECL(ScalePointZ);
        ALGORITHM_PARAMETER_DECL(LogScale);
        ALGORITHM_PARAMETER_DECL(LogScaleX);
        ALGORITHM_PARAMETER_DECL(LogScaleY);
        ALGORITHM_PARAMETER_DECL(LogScaleZ);
        ALGORITHM_PARAMETER_DECL(RotatePointX);
        ALGORITHM_PARAMETER_DECL(RotatePointY);
        ALGORITHM_PARAMETER_DECL(RotatePointZ);
        ALGORITHM_PARAMETER_DECL(RotateAxisX);
        ALGORITHM_PARAMETER_DECL(RotateAxisY);
        ALGORITHM_PARAMETER_DECL(RotateAxisZ);
        ALGORITHM_PARAMETER_DECL(RotateTheta);
        ALGORITHM_PARAMETER_DECL(ShearVectorX);
        ALGORITHM_PARAMETER_DECL(ShearVectorY);
        ALGORITHM_PARAMETER_DECL(ShearVectorZ);
        ALGORITHM_PARAMETER_DECL(ShearPlaneA);
        ALGORITHM_PARAMETER_DECL(ShearPlaneB);
        ALGORITHM_PARAMETER_DECL(ShearPlaneC);
        ALGORITHM_PARAMETER_DECL(ShearPlaneD);
        ALGORITHM_PARAMETER_DECL(FieldMapX);
        ALGORITHM_PARAMETER_DECL(FieldMapY);
        ALGORITHM_PARAMETER_DECL(FieldMapZ);
        ALGORITHM_PARAMETER_DECL(PermuteX);
        ALGORITHM_PARAMETER_DECL(PermuteY);
        ALGORITHM_PARAMETER_DECL(PermuteZ);
        ALGORITHM_PARAMETER_DECL(UniformScale);
        ALGORITHM_PARAMETER_DECL(ResizeSeparably);
        ALGORITHM_PARAMETER_DECL(IgnoreChanges);
      }
    }
  }
namespace Modules {
namespace Math {

  class SCISHARE CreateGeometricTransform : public SCIRun::Dataflow::Networks::Module,
    public Has1InputPort<MatrixPortTag>,
    public Has2OutputPorts<MatrixPortTag, GeometryPortTag>
  {
  public:
    CreateGeometricTransform();
    virtual void execute() override;

    void setStateDefaults() override;

    INPUT_PORT(0, InputMatrix, Matrix);
    OUTPUT_PORT(0, OutputMatrix, Matrix);
    OUTPUT_PORT(1, Widget, GeometryObject);
    MODULE_TRAITS_AND_INFO(ModuleHasUI)

  private:
    Core::Datatypes::MatrixHandle omatrixH_;
    Core::Geometry::Transform composite_trans_;
    Core::Geometry::Transform latest_trans_;
  };
}}}

#endif
