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


#ifndef CORE_ALGORITHMS_FIELDS_MERGEFIELDS_JOINFIELDS_H
#define CORE_ALGORITHMS_FIELDS_MERGEFIELDS_JOINFIELDS_H 1

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/Fields/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {
      namespace Fields {

        ALGORITHM_PARAMETER_DECL(merge_nodes);
        ALGORITHM_PARAMETER_DECL(merge_elems);
        ALGORITHM_PARAMETER_DECL(tolerance);
        ALGORITHM_PARAMETER_DECL(match_node_values);
        ALGORITHM_PARAMETER_DECL(make_no_data);
        ALGORITHM_PARAMETER_DECL(ForcePointCloud);

        class SCISHARE JoinFieldsAlgo : public AlgorithmBase
        {
        public:
          JoinFieldsAlgo();
          bool runImpl(const FieldList& input, FieldHandle& output) const;
          AlgorithmOutput run(const AlgorithmInput& input) const override;
        };

}}}}

#endif
