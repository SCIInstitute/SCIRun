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

class SCISHARE GetFieldDataAlgo : public AlgorithmBase
{
  public:
    GetFieldDataAlgo();
    
    static AlgorithmOutputName OutputMatrix;
    
    Datatypes::DenseMatrixHandle GetScalarFieldDataV(FieldHandle& input) const;
    Datatypes::DenseMatrixHandle GetVectorFieldDataV(FieldHandle& input) const;
    Datatypes::DenseMatrixHandle GetTensorFieldDataV(FieldHandle& input) const;
    Datatypes::DenseMatrixHandle run(FieldHandle input_field) const; 
    virtual AlgorithmOutput run_generic(const AlgorithmInput& input) const; 
    #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    bool GetScalarFieldDataV(AlgoBase *algo, FieldHandle& input, NrrdDataHandle& output) const;
    bool GetVectorFieldDataV(AlgoBase *algo, FieldHandle& input, NrrdDataHandle& output) const;
    bool GetTensorFieldDataV(AlgoBase *algo, FieldHandle& input, NrrdDataHandle& output) const;
    #endif
};

}}}}
#endif
