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

#ifndef CORE_ALGORITHMS_FIELDS_FIELDDATA_GETFIELDDATA_H
#define CORE_ALGORITHMS_FIELDS_FIELDDATA_GETFIELDDATA_H 1


#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/Fields/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {
      namespace Fields {

        ALGORITHM_PARAMETER_DECL(CalcMatrix);
        ALGORITHM_PARAMETER_DECL(CalcNrrd);

        class SCISHARE GetFieldDataAlgo : public AlgorithmBase
        {
        public:
          GetFieldDataAlgo();
                
          Datatypes::DenseMatrixHandle run(FieldHandle input) const;
          NrrdDataHandle runNrrd(FieldHandle input) const;

          virtual AlgorithmOutput run_generic(const AlgorithmInput& input) const override;

        private:
          template <class MatrixReturnType>
          boost::shared_ptr<MatrixReturnType> runImpl(FieldHandle input) const;
          template <class MatrixReturnType>
          bool GetScalarFieldDataV(FieldHandle input, boost::shared_ptr<MatrixReturnType>& output) const;
          template <class MatrixReturnType>
          bool GetVectorFieldDataV(FieldHandle input, boost::shared_ptr<MatrixReturnType>& output) const;
          template <class MatrixReturnType>
          bool GetTensorFieldDataV(FieldHandle input, boost::shared_ptr<MatrixReturnType>& output) const;
        };

      }
    }
  }
}
#endif
