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


#ifndef ALGORITHMS_MATH_CalculateInsideWhichFieldAlgorithm_H
#define ALGORITHMS_MATH_CalculateInsideWhichFieldAlgorithm_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Algorithms/Field/share.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/CalculateMeshCenterAlgo.h>


namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Fields {


  //ALGORITHM_PARAMETER_DECL(SamplingScheme);
  ALGORITHM_PARAMETER_DECL(ChangeOutsideValue);
//ALGORITHM_PARAMETER_DECL(OutsideValue);
  ALGORITHM_PARAMETER_DECL(StartValue);
  //ALGORITHM_PARAMETER_DECL(OutputType);
  ALGORITHM_PARAMETER_DECL(DataLocation);

  class SCISHARE CalculateInsideWhichFieldAlgorithm : public AlgorithmBase
  {

  public:
    CalculateInsideWhichFieldAlgorithm();
    //static const AlgorithmInputName InputField;

    FieldHandle run(FieldHandle input,const FieldList& objField)const;

    virtual AlgorithmOutput run(const AlgorithmInput &) const override;


  };

}}}}

#endif
