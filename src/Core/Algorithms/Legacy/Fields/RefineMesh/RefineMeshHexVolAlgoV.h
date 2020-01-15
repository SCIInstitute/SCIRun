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


#ifndef CORE_ALGORITHMS_FIELDS_REFINEMESH_REFINEMESHHEXVOLALGOV_H
#define CORE_ALGORITHMS_FIELDS_REFINEMESH_REFINEMESHHEXVOLALGOV_H 1

// Datatypes that the algorithm uses
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Algorithms/Legacy/Fields/RefineMesh/EdgePairHash.h>

// Base class for algorithm
#include <Core/Algorithms/Base/AlgorithmBase.h>

// for Windows support
#include <Core/Algorithms/Legacy/Fields/share.h>

namespace SCIRun{
  namespace Core{
    namespace Algorithms{
      namespace Fields{

        class SCISHARE RefineMeshHexVolAlgoV : public AlgorithmBase
        {
        public:
          RefineMeshHexVolAlgoV();

          bool runImpl(FieldHandle input, FieldHandle& output, bool convex, const std::string& select, double isoval) const;
          AlgorithmOutput run(const AlgorithmInput& input) const override;

        private:
          VMesh::Node::index_type add_point(VMesh *refined,
            VMesh::Node::array_type &nodes,
            const int *reorder,
            unsigned int a,
            unsigned int b,
            double factor,
            std::vector<double>& ivalues,
            int basis_order) const;

          VMesh::Node::index_type add_point_convex(VMesh *refined,
            VMesh::Node::array_type &nodes,
            const Geometry::Point& coordsp,
            std::vector<double>& ivalues,
            int basis_order) const;

          VMesh::Node::index_type add_point_convex(VMesh *refined,
            VMesh::Node::array_type &nodes,
            const int *reorder,
            VMesh::index_type a,
            VMesh::index_type b,
            std::vector<double>& ivalues,
            int basis_order) const;


          void dice(VMesh *refined,
            edge_hash_type &emap,
            VMesh::Node::array_type nodes,
            VMesh::index_type index,
            VMesh::mask_type mask,
            std::vector<double>& ivalues,
            std::vector<double>& evalues,
            double vv,
            int basis_order) const;

          VMesh::Node::index_type lookup(VMesh *refined,
            edge_hash_type &edgemap,
            VMesh::Node::array_type &nodes,
            const int *reorder,
            VMesh::index_type a,
            VMesh::index_type b,
            double factor,
            std::vector<double>& ivalues,
            int basis_order) const;
          VMesh::Node::index_type lookup_convex(VMesh *refined,
            edge_hash_type &edgemap,
            VMesh::Node::array_type &onodes,
            const int *reorder,
            VMesh::index_type a,
            VMesh::index_type b,
            std::vector<double>& ivalues,
            int basis_order) const;
        };
      }
    }
  }
}

#endif
