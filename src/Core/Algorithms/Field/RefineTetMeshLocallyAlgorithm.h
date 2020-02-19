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


#ifndef CORE_ALGORITHMS_FIELD_RefineTetMeshLocally_H
#define CORE_ALGORITHMS_FIELD_RefineTetMeshLocally_H 1

// Datatypes that the algorithm uses
#include <Core/Datatypes/DatatypeFwd.h>

// Base class for algorithm
#include <Core/Algorithms/Base/AlgorithmBase.h>

#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>

// for Windows support
#include <Core/Algorithms/Field/share.h>

namespace SCIRun{
		namespace Core{
				namespace Algorithms{
						namespace Fields{

ALGORITHM_PARAMETER_DECL(RefineTetMeshLocallyIsoValue);
ALGORITHM_PARAMETER_DECL(RefineTetMeshLocallyEdgeLength);
ALGORITHM_PARAMETER_DECL(RefineTetMeshLocallyVolume);
ALGORITHM_PARAMETER_DECL(RefineTetMeshLocallyDihedralAngleSmaller);
ALGORITHM_PARAMETER_DECL(RefineTetMeshLocallyDihedralAngleBigger);
ALGORITHM_PARAMETER_DECL(RefineTetMeshLocallyDoNoSplitSurfaceTets);
ALGORITHM_PARAMETER_DECL(RefineTetMeshLocallyRadioButtons);
ALGORITHM_PARAMETER_DECL(RefineTetMeshLocallyCounterClockWiseOrdering);
ALGORITHM_PARAMETER_DECL(RefineTetMeshLocallyUseModuleInputField);
ALGORITHM_PARAMETER_DECL(RefineTetMeshLocallyMaxNumberRefinementIterations);

class SCISHARE RefineTetMeshLocallyAlgorithm : public AlgorithmBase
{
  public:
    RefineTetMeshLocallyAlgorithm();
    FieldHandle RefineMesh(FieldHandle input, Datatypes::SparseRowMatrixHandle cut_edges) const;
    bool runImpl(FieldHandle input, FieldHandle& output) const;
  private:
    static const int max_number_new_tets;
    static const int number_edges;
    static const int number_nodes;
    static const int number_cases;
    static const int number_faces;
    static const int code_to_split;
    static const int code_not_to_split;
    static const int EdgeLookup[6][3];
    static int CaseLookup[63][6];
    static const int Case1Lookup[2][4];
    static const int Case2aLookup[3][4];
    static const int Case2bLookup[4][4];
    static const int Case3aLookup[4][4];
    static const int Case3bLookup[4][4];
    static const int Case3cLookup[5][4];
    static const int Case3cNonNegativeLookup[5][4];
    static const int Case4aLookup[6][4];
    static const int Case4aNonNegativeLookup[6][4];
    static const int Case4bLookup[6][4];
    static const int Case5Lookup[7][4];
    static const int Case6Lookup[12][4];
    static const int NumberTets[12];
    Datatypes::SparseRowMatrixHandle ChoseEdgesToCut(FieldHandle input, const std::vector<long>& elems_to_split, VMesh* field_boundary) const;
    std::vector<int> SelectMeshElements(FieldHandle input, int choose_refinement_option, int& count) const;
    virtual AlgorithmOutput run(const AlgorithmInput& input) const override;
    std::vector<int> maxi(const std::vector<double>& input_vec) const;
    std::vector<int> getEdgeCoding(int pos) const;
    std::vector<double> getEdgeLengths(Geometry::Point p1, Geometry::Point p2, Geometry::Point p3, Geometry::Point p4) const;
};
}}}}

#endif
