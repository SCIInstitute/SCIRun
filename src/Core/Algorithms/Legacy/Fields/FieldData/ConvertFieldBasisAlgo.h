/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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


#ifndef CORE_ALGORITHMS_FIELDS_FIELDDATA_CONVERT_FIELD_BASIS_ALGO_H
#define CORE_ALGORITHMS_FIELDS_FIELDDATA_CONVERT_FIELD_BASIS_ALGO_H 1

// Datatypes that the algorithm uses
#include <Core/Datatypes/DatatypeFwd.h> 
#include <Core/Algorithms/Base/AlgorithmBase.h> 

// Base class for algorithm
//#include <Core/Algorithms/Util/AlgoBase.h>

// for Windows support
#include <Core/Algorithms/Legacy/Fields/share.h>

namespace SCIRun{
		namespace Core{
				namespace Algorithms{
						namespace Fields{

		ALGORITHM_PARAMETER_DECL(OutputBasis);
		ALGORITHM_PARAMETER_DECL(InputBasis);
		ALGORITHM_PARAMETER_DECL(InputName);
								
class SCISHARE ConvertFieldBasisAlgo : public AlgorithmBase
{
  public:
    /// Set defaults
    ConvertFieldBasisAlgo();
    //{
    //  /// keep scalar type defines whether we convert to double or not
    //  add_option("method","value-mean","min|max|median|value-mean|geom-mean|sum|integral|volthreshold");
    //  add_scalar("threshold",0.5);
    //}

		virtual AlgorithmOutput run_generic(const AlgorithmInput& input) const override;
		bool runImpl(FieldHandle input, FieldHandle& output) const;
		bool runImpl(FieldHandle input, FieldHandle& output, Datatypes::MatrixHandle& mapping) const;
		

    //bool run(std::vector<FieldHandle>& input, MatrixHandle& output);
    //bool run(FieldHandle input, MatrixHandle& output);
};

						}}}}

#endif

