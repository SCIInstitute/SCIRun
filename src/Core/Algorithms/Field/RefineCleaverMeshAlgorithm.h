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

#ifndef CORE_ALGORITHMS_FIELD_REFINECLEAVERMESH_H
#define CORE_ALGORITHMS_FIELD_REFINECLEAVERMESH_H 1

// Datatypes that the algorithm uses
#include <Core/Datatypes/DatatypeFwd.h> 

// Base class for algorithm
#include <Core/Algorithms/Base/AlgorithmBase.h>

// for Windows support
#include <Core/Algorithms/Field/share.h>

#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
namespace SCIRun{
		namespace Core{
				namespace Algorithms{
						namespace Fields{

ALGORITHM_PARAMETER_DECL(RefineCleaverMeshIsoValue);
ALGORITHM_PARAMETER_DECL(RefineCleaverMeshEdgeLength);
ALGORITHM_PARAMETER_DECL(RefineCleaverMeshVolume);
ALGORITHM_PARAMETER_DECL(RefineCleaverMeshDihedralAngleSmaller);
ALGORITHM_PARAMETER_DECL(RefineCleaverMeshDihedralAngleBigger);
ALGORITHM_PARAMETER_DECL(RefineCleaverMeshDoNoSplitSurfaceTets);
ALGORITHM_PARAMETER_DECL(RefineCleaverMeshRadioButtons);

class SCISHARE RefineCleaverMeshAlgorithm : public AlgorithmBase
{
  public:  
    RefineCleaverMeshAlgorithm();
    std::vector<bool> SelectMeshElements(FieldHandle input, Datatypes::DenseMatrixHandle matrix, int choose_refinement_option, int& count) const;
    bool runImpl(FieldHandle input, Datatypes::DenseMatrixHandle matrix, FieldHandle& output) const;
    FieldHandle RefineMesh(FieldHandle input, std::vector<long> elems_to_split) const; 
    bool ComputeEdgeMidPoint(int pos, Geometry::Point p1, Geometry::Point p2, Geometry::Point p3, Geometry::Point p4, Geometry::Point& output) const;
    AlgorithmOutput run_generic(const AlgorithmInput& input) const; 
    bool SplitTet(VMesh::Node::array_type cell, int edge_pos, Datatypes::DenseMatrixHandle two_new_tets, long& node_count) const;
    std::vector<int> maxi(std::vector<double> input_vec) const;
    std::vector<int> getEdgeCoding(int pos) const;
    std::vector<double> getEdgeLengths(Geometry::Point p1, Geometry::Point p2, Geometry::Point p3, Geometry::Point p4) const;
};
}}}}

#endif
