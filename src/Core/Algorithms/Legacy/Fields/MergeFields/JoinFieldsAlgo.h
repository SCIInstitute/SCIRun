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


#ifndef CORE_ALGORITHMS_FIELDS_MERGEFIELDS_JOINFIELDS_H
#define CORE_ALGORITHMS_FIELDS_MERGEFIELDS_JOINFIELDS_H 1

<<<<<<< HEAD:src/Core/Algorithms/Legacy/Fields/MergeFields/JoinFields.h
#include <Core/Datatypes/Field.h>
#include <vector>

#include <Core/Algorithms/Util/AlgoBase.h>
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

class SCISHARE JoinFieldsAlgo : public AlgoBase
{
  public:
    // Algorithm defaults
    JoinFieldsAlgo()
    {
      /// Merge duplicate nodes?
      add_bool("merge_nodes",true);
      /// Merge duplicate elements?
      add_bool("merge_elems",false);
      /// Tolerance for merging duplicate nodes?
      add_scalar("tolerance",1e-6);
      /// Only merge nodes whose value is the same
      add_bool("match_node_values",false);
      /// Create a field with no data
      add_bool("make_no_data",false);
    }
    bool run(std::vector<SCIRun::FieldHandle>& input, SCIRun::FieldHandle& output);   
};

}
=======
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/Fields/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {
      namespace Fields {

        class SCISHARE JoinFieldsAlgo : public AlgorithmBase
        {
        public:
          JoinFieldsAlgo();
          bool runImpl(const FieldList& input, FieldHandle& output) const;

          static AlgorithmParameterName MergeNodes;
          static AlgorithmParameterName MergeElems;
          static AlgorithmParameterName Tolerance;
          static AlgorithmParameterName MatchNodeValues;
          static AlgorithmParameterName MakeNoData;
          static AlgorithmInputName InputFields;

          virtual AlgorithmOutput run_generic(const AlgorithmInput& input) const override;
        };

}}}}
>>>>>>> ead7d0073d5f955de2d7e3e7d1e80122cf8aad18:src/Core/Algorithms/Legacy/Fields/MergeFields/JoinFieldsAlgo.h

#endif
