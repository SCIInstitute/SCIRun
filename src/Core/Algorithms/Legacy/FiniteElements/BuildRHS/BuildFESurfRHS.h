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


//@file BuildFESurfRHS.h
///@brief This module computes
///
///@author
/// ported by Moritz Dannhauer (10/29/2017) from SCIRun4
///
///@details
///
///
///
/// Input:  Output:



#ifndef CORE_ALGORITHMS_FINITEELEMENTS_BUILDFESURFRHS_H
#define CORE_ALGORITHMS_FINITEELEMENTS_BUILDFESURFRHS_H 1

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/FiniteElements/share.h>


namespace SCIRun {
	namespace Core {
		namespace Algorithms {
			namespace FiniteElements {

ALGORITHM_PARAMETER_DECL(InputField);
ALGORITHM_PARAMETER_DECL(BoundaryField);
ALGORITHM_PARAMETER_DECL(RHSMatrix);

class SCISHARE BuildFESurfRHSAlgo : public AlgorithmBase
{

  public:
   BuildFESurfRHSAlgo() {}
   bool run(FieldHandle input, FieldHandle& output, Datatypes::MatrixHandle& mat_output) const;
   virtual AlgorithmOutput run(const AlgorithmInput &) const;
};

}}}}

#endif
