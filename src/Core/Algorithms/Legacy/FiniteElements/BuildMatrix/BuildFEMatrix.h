/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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


#ifndef CORE_ALGORITHMS_FINITEELEMENTS_BUILDFEMATRIX_H
#define CORE_ALGORITHMS_FINITEELEMENTS_BUILDFEMATRIX_H 1

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <vector>
#include <Core/Algorithms/Legacy/FiniteElements/share.h>

namespace SCIRun {
	namespace Core {
		namespace Algorithms {
			namespace FiniteElements {

class SCISHARE BuildFEMatrixAlgo : public AlgorithmBase
{
  public:
    enum { AUTO = -1 };
  
    static AlgorithmParameterName NumProcessors;
    static AlgorithmParameterName ForceSymmetry;
    static AlgorithmParameterName GenerateBasis;

    static AlgorithmInputName Conductivity_Table;
    static AlgorithmOutputName Stiffness_Matrix;

    BuildFEMatrixAlgo() : generation_(0)
    {
      // Number of processors to use
      addParameter(NumProcessors, AUTO);
      
      // Whether to force strict symmetry of the matrix
      // Averages symmetric components
      addParameter(ForceSymmetry, false);

      // Store intermediate results to speed up computation for
      // for instance conductivity search
      // This option only works for an indexed conductivity table
      addParameter(GenerateBasis, false);
    }

    bool run(FieldHandle input,
             Datatypes::DenseMatrixHandle ctable,
             Datatypes::SparseRowMatrixHandle& output) const;
  
    virtual AlgorithmOutput run(const AlgorithmInput &) const;
private:
  mutable int generation_;
  mutable std::vector<std::vector<double> > basis_values_;
  mutable Datatypes::SparseRowMatrixHandle basis_fematrix_;
};

}}}}

#endif 
