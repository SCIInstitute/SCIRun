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


#ifndef ALGORITHMS_MATH_BiotSavartSolverAlgorithm_H
#define ALGORITHMS_MATH_BiotSavartSolverAlgorithm_H

#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Matrix.h>

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/BrainStimulator/share.h>

///@file BiotSavartSolverAlgorithm
///@brief
///
///
///@author
/// Implementation: Petar Petrov for SCIRun 4.7
/// Converted to SCIRun5 by Moritz Dannhauer
///@details
///
///

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace BrainStimulator {

 ALGORITHM_PARAMETER_DECL(Mesh);
 ALGORITHM_PARAMETER_DECL(Coil);
 ALGORITHM_PARAMETER_DECL(VectorBField);
 ALGORITHM_PARAMETER_DECL(VectorAField);
 ALGORITHM_PARAMETER_DECL(OutType);

  class SCISHARE BiotSavartSolverAlgorithm : public AlgorithmBase
  {
  public:
    BiotSavartSolverAlgorithm()
    {
      addParameter(Parameters::OutType, 0);
    }
    AlgorithmOutput run(const AlgorithmInput& input) const override;
    bool run(FieldHandle mesh, FieldHandle coil, Datatypes::DenseMatrixHandle& outdata, int outtype) const;
  };

}}}}

#endif
