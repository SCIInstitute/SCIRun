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


#ifndef ALGORITHMS_MATH_INTERFACEWITHCLEAVER2_H
#define ALGORITHMS_MATH_INTERFACEWITHCLEAVER2_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Algorithms/Field/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Fields {

  //Cleaver parameters: Scaling = Sampling Rate, Multiplier = Feature Scaling
  ALGORITHM_PARAMETER_DECL(Verbose);
  ALGORITHM_PARAMETER_DECL(VolumeScaling);
  ALGORITHM_PARAMETER_DECL(VolumeMultiplier);
  ALGORITHM_PARAMETER_DECL(Lipschitz);
  ALGORITHM_PARAMETER_DECL(Padding);
  ALGORITHM_PARAMETER_DECL(AlphaLong);
  ALGORITHM_PARAMETER_DECL(AlphaShort);
  ALGORITHM_PARAMETER_DECL(SimpleMode);
  ALGORITHM_PARAMETER_DECL(MeshMode);
  ALGORITHM_PARAMETER_DECL(ReverseJacobians);

  class SCISHARE InterfaceWithCleaver2Algorithm : public AlgorithmBase
  {
  public:
    InterfaceWithCleaver2Algorithm();
    static const AlgorithmInputName SizingField;
    static const AlgorithmInputName BackgroundField;
    static const AlgorithmOutputName SizingFieldUsed;
    static const AlgorithmOutputName BackgroundFieldUsed;

    AlgorithmOutput runImpl(const FieldList& input, FieldHandle backgroundMesh = nullptr, FieldHandle sizingField = nullptr) const;
    virtual AlgorithmOutput run(const AlgorithmInput &) const override;
  };

}}}}

#endif
