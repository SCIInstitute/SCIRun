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

   Author:              Moritz Dannhauer
   Last Modification:   February 26 2016
   TODO:                implement in parallel execution (Dan White), transient GUI variables
   Related Literature:  Ruprecht and Mueller, 'A Scheme for Edge-based Adaptive Tetrahedron Subdivision', 1994
                        The implementation contains a modified version published in Thomson and Pebay
                        'Embarrassingly parallel mesh refinement by edge subdivision', 2006
                        and differs in the fact that here its not a communication-free approach (tet neighbors
                        are determined during refinement and that costs more
                        computation time and memory) but it does generate significantly less new element
                        nodes/elements then their approach which is a major factor if dealing with large meshes
                        such as from Cleaver1. This implementation is simplified in way that edge splitting ambiguities
                        (several sub cases for each main case) are not treated as such rather than based on their
                        proposed edge length criteria for general tetrahedra. Here, we are using the longest edge
                        approach as a simple criteria to slow down element quality decay during multiple refinement steps.
                        e.g., if there are 3 edges that are equally long in a particular tetrahedra it will get
                        split up at those 3 edges.
   Requirements:        if dealing with refined Cleaver1 meshes this implementation requires lots of RAM memory (>= 16 GB).
*/


#include <Core/Algorithms/Legacy/Fields/DomainFields/SplitFieldByDomainAlgo.h>
#include <Core/Algorithms/Legacy/Fields/MergeFields/JoinFieldsAlgo.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundaryAlgo.h>
#include <Core/Algorithms/Field/RefineTetMeshLocallyAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Logging/Log.h>
#include <vector>
#include <iterator>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;

ALGORITHM_PARAMETER_DEF(Fields, RefineTetMeshLocallyIsoValue);
ALGORITHM_PARAMETER_DEF(Fields, RefineTetMeshLocallyEdgeLength);
ALGORITHM_PARAMETER_DEF(Fields, RefineTetMeshLocallyVolume);
ALGORITHM_PARAMETER_DEF(Fields, RefineTetMeshLocallyDihedralAngleSmaller);
ALGORITHM_PARAMETER_DEF(Fields, RefineTetMeshLocallyDihedralAngleBigger);
ALGORITHM_PARAMETER_DEF(Fields, RefineTetMeshLocallyDoNoSplitSurfaceTets);
ALGORITHM_PARAMETER_DEF(Fields, RefineTetMeshLocallyRadioButtons);
ALGORITHM_PARAMETER_DEF(Fields, RefineTetMeshLocallyCounterClockWiseOrdering);
ALGORITHM_PARAMETER_DEF(Fields, RefineTetMeshLocallyUseModuleInputField);
ALGORITHM_PARAMETER_DEF(Fields, RefineTetMeshLocallyMaxNumberRefinementIterations);

const int RefineTetMeshLocallyAlgorithm::code_to_split = 1;
const int RefineTetMeshLocallyAlgorithm::code_not_to_split = 0;
const int RefineTetMeshLocallyAlgorithm::number_edges = 6;
const int RefineTetMeshLocallyAlgorithm::number_faces = 4;
const int RefineTetMeshLocallyAlgorithm::number_nodes = 4;
const int RefineTetMeshLocallyAlgorithm::number_cases = 64;
const int RefineTetMeshLocallyAlgorithm::max_number_new_tets = 12; /// ... per tet
/*

 Tetrahedral element definition:

     2
    /\
   / |\
  /  | \
1 ------ 3
  \  | /
   \ |/
    \/
    0

 Edges definition:
 I   = 0-1
 II  = 1-2
 III = 0-2
 IV  = 0-3
 V   = 1-3
 VI  = 2-3


 Additional edge split points:
 I   = 4
 II  = 5
 III = 6
 IV  = 7
 V   = 5
 VI  = 9

 */

const int RefineTetMeshLocallyAlgorithm::NumberTets[12] =
{
  2, 3, 4, 4, 4, 5, 6, 6, 7, 12, 5, 6
};


const int RefineTetMeshLocallyAlgorithm::EdgeLookup[6][3] =
{
  { 4, 0, 1 },
  { 5, 1, 2 },
  { 6, 0, 2 },
  { 7, 0, 3 },
  { 8, 1, 3 },
  { 9, 2, 3 }
};

const int RefineTetMeshLocallyAlgorithm::Case1Lookup[2][4] =
{
  { 0, 4, 2, 3 },
  { 4, 1, 2, 3 }
};

/// for this case 2a - 2a^ubeta was chosen from Thompson et al.
const int RefineTetMeshLocallyAlgorithm::Case2aLookup[3][4] =
{
  { 0, 4, 5, 3 },
  { 0, 5, 2, 3 },
  { 4, 1, 5, 3 }
};

const int RefineTetMeshLocallyAlgorithm::Case2bLookup[4][4] =
{
  { 0, 4, 9, 3 },
  { 4, 1, 9, 3 },
  { 0, 4, 2, 9 },
  { 4, 1, 2, 9 }
};

/// for this case 3a - 3a^u was chosen from Thompson et al.
const int RefineTetMeshLocallyAlgorithm::Case3aLookup[4][4] =
{
  { 0, 4, 6, 7 },
  { 4, 1, 2, 3 },
  { 4, 2, 6, 3 },
  { 4, 6, 7, 3 }
};

const int RefineTetMeshLocallyAlgorithm::Case3bLookup[4][4] =
{
  { 0, 7, 4, 2 },
  { 4, 7, 8, 2 },
  { 4, 8, 1, 2 },
  { 7, 3, 8, 2 }
};

/// for this case 3c - 3c^ualpha was chosen from Thompson et al.
const int RefineTetMeshLocallyAlgorithm::Case3cLookup[5][4] =
{
  { 4, 2, 7, 5 },
  { 4, 2, 0, 7 },
  { 4, 1, 5, 3 },
  { 5, 7, 4, 3 },
  { 5, 2, 7, 3 }
};

/// this case has been added to avoid negative volume (tet ordering), other than that its identical to case 3c
const int RefineTetMeshLocallyAlgorithm::Case3cNonNegativeLookup[5][4] =
{
  { 4, 7, 2, 5 },
  { 4, 0, 2, 7 },
  { 4, 5, 1, 3 },
  { 5, 4, 7, 3 },
  { 5, 7, 2, 3 }
};

/// for this case 4a - 4a^ualpha was chosen from Thompson et al.
const int RefineTetMeshLocallyAlgorithm::Case4aLookup[6][4] =
{
  { 7, 8, 9, 3 },
  { 6, 9, 7, 8 },
  { 6, 0, 1, 8 },
  { 6, 7, 0, 8 },
  { 1, 2, 6, 9 },
  { 1, 6, 8, 9 }
};

/// this case has been added to avoid negative volume (tet ordering), other than that its identical to case 4a
const int RefineTetMeshLocallyAlgorithm::Case4aNonNegativeLookup[6][4] =
{
  { 7, 9, 8, 3 },
  { 6, 7, 9, 8 },
  { 6, 1, 0, 8 },
  { 6, 0, 7, 8 },
  { 1, 6, 2, 9 },
  { 1, 8, 6, 9 }
};
/// for this case 4b - 4b^ualpha was chosen from Thompson et al.
const int RefineTetMeshLocallyAlgorithm::Case4bLookup[6][4] =
{
  { 6, 8, 1, 5 },
  { 6, 8, 0, 1 },
  { 6, 7, 0, 8 },
  { 8, 7, 3, 2 },
  { 6, 8, 5, 2 },
  { 6, 8, 2, 7 }
};

/// for this case 5 - 5^ualpha was chosen from Thompson et al.
const int RefineTetMeshLocallyAlgorithm::Case5Lookup[7][4] =
{
  { 7, 8, 9, 3 },
  { 6, 5, 2, 9 },
  { 5, 7, 1, 8 },
  { 5, 7, 0, 1 },
  { 5, 7, 6, 0 },
  { 5, 7, 9, 6 },
  { 7, 8, 5, 9 }
};

/// this case has been adjusted as it was published to avoid one "split iteration", since the splitting in Thompson
/// creates one tet edge that has original tet edge length
//const int RefineTetMeshLocallyAlgorithm::Case6Lookup[8][4] =
const int RefineTetMeshLocallyAlgorithm::Case6Lookup[max_number_new_tets][4] =
{
  /*{7, 8, 9, 3},
  {6, 5, 2, 9},
  {4, 1, 5, 8},
  {0, 4, 6, 7},
  {6, 4, 5, 8},
  {6, 5, 9, 8},
  {6, 9, 7, 8},
  {6, 7, 4, 8}*/
  { 7, 8, 9, 3 },
  { 6, 5, 2, 9 },
  { 4, 1, 5, 8 },
  { 0, 4, 6, 7 },
  { 6, 10, 4, 5 },
  { 6, 4, 10, 7 },
  { 6, 7, 10, 9 },
  { 6, 9, 10, 5 },
  { 10, 5, 8, 4 },
  { 10, 8, 5, 9 },
  { 10, 8, 9, 7 },
  { 10, 8, 7, 4 }
};

int RefineTetMeshLocallyAlgorithm::CaseLookup[number_cases - 1][6] =
{
  { 1, 2, 3, 0, 1, 1 }, /// case  1: edge 2-3 is cut
  { 2, 1, 3, 2, 0, 1 }, /// case  1: edge 1-3 is cut
  { 3, 1, 3, 2, 0, 2 }, /// case 2a: edge 1-3, 2-3 is cut
  { 4, 3, 0, 2, 1, 1 }, /// case  1: edge 0-3 is cut
  { 5, 2, 3, 0, 1, 2 }, /// case 2a: edge 0-3, 2-3 is cut
  { 6, 0, 3, 1, 2, 2 }, /// case 2a: edge 0-3, 1-3 is cut
  { 7, 3, 0, 2, 1, 4 }, /// case 3a: edge 0-2, 0-3, 2-3 is cut
  { 8, 0, 2, 3, 1, 1 }, /// case  1: edge 0-2 is cut
  { 9, 0, 2, 3, 1, 2 }, /// case 2a: edge 0-2, 2-3 is cut
  { 10, 1, 3, 2, 0, 3 }, /// case 2b: edge 0-2, 1-3 is cut
  { 11, 3, 2, 0, 1, 11 }, /// case 3c: edge 0-2, 1-3, 2-3 is cut, this just an additional subcase to avoid negative volumes (otherwise identical with case 3c)
  { 12, 3, 0, 2, 1, 2 }, /// case 2a: edge 0-2, 0-3 is cut
  { 13, 3, 2, 1, 0, 5 }, /// case 3b: edge 0-2, 0-3, 2-3 is cut
  { 14, 3, 0, 2, 1, 6 }, /// case 3c: edge 0-2, 0-3, 1-3 is cut
  { 15, 0, 1, 2, 3, 7 }, /// case 4a: edge 0-2, 0-3, 1-3, 2-3 is cut, no recoding needed for this case
  { 16, 1, 2, 0, 3, 1 }, /// case  1: edge 1-2 is cut
  { 17, 3, 2, 1, 0, 2 }, /// case 2a: edge 1-2, 2-3 is cut
  { 18, 2, 1, 3, 0, 2 }, /// case 2a: edge 1-2, 1-3 is cut
  { 19, 1, 2, 0, 3, 5 }, /// case 3b: edge 1-2, 1-3, 2-3 is cut
  { 20, 3, 0, 2, 1, 3 }, /// case 2b: edge 0-3, 1-2 is cut
  { 21, 3, 2, 1, 0, 6 }, /// case 3c: edge 0-3, 1-2, 2-3 is cut
  { 22, 3, 1, 2, 0, 11 }, /// case 3c: edge 0-3, 1-2, 1-3 is cut, this just an additional subcase to avoid negative volumes (otherwise identical with case 3c)
  { 23, 2, 0, 1, 3, 7 }, /// case 4a: edge 0-3, 1-2, 1-3, 2-3 is cut
  { 24, 1, 2, 0, 3, 2 }, /// case 2a: edge 0-2, 1-2 is cut
  { 25, 2, 1, 3, 0, 4 }, /// case 3a: edge 0-2, 1-2, 2-3 is cut
  { 26, 1, 2, 0, 3, 6 }, /// case 3c: edge 0-2, 1-2, 1-3 is cut
  { 27, 1, 0, 3, 2, 7 }, /// case 4a: edge 0-3, 1-2, 1-3, 2-3 is cut
  { 28, 2, 0, 3, 1, 11 }, /// case 3c: edge 0-2, 0-3, 1-2, this just an additional subcase to avoid negative volumes (otherwise identical with case 3c)
  { 29, 3, 1, 0, 2, 7 }, /// case 4a: edge 0-2, 0-3, 1-2, 2-3 is cut
  { 30, 0, 1, 2, 3, 8 }, /// case 4b: edge 0-2, 0-3, 1-2, 1-3 is cut
  { 31, 0, 1, 2, 3, 9 }, /// case  5: edge 0-2, 0-3, 1-2, 1-3, 2-3 is cut
  { 32, 0, 1, 2, 3, 1 }, /// case  1: edge 0-1 is cut, no recoding needed for this case
  { 33, 0, 1, 2, 3, 3 }, /// case 2b: edge 0-1, 2-3 is cut
  { 34, 3, 1, 0, 2, 2 }, /// case 2a: edge 0-1, 1-3 is cut
  { 35, 1, 3, 2, 0, 6 }, /// case 3c: edge 0-1, 1-3, 2-3 is cut
  { 36, 1, 0, 3, 2, 2 }, /// case 2a: edge 0-1, 0-3 is cut
  { 37, 0, 3, 2, 1, 11 }, /// case 3c: edge 0-1, 0-3, 2-3 is cut, this just an additional subcase to avoid negative volumes (otherwise identical with case 3c)
  { 38, 0, 1, 2, 3, 5 }, /// case 3b: edge 0-1, 0-3, 1-3 is cut, no recoding needed for this case
  { 39, 1, 2, 0, 3, 7 }, /// case 4a: edge 0-1, 0-3, 1-3, 2-3 is cut
  { 40, 2, 0, 1, 3, 2 }, /// case 2a: edge 0-1, 0-2 is cut
  { 41, 0, 2, 3, 1, 6 }, /// case 3c: edge 0-1, 0-2, 2-3 is cut
  { 42, 1, 0, 2, 3, 11 }, /// case 3c: edge 0-1, 0-2, 1-3 is cut, this just an additional subcase to avoid negative volumes (otherwise identical with case 3c)
  { 43, 3, 0, 2, 1, 8 }, /// case 4b: edge 0-1, 0-2, 1-3, 2-3 is cut
  { 44, 0, 1, 2, 3, 4 }, /// case 3a, edge 0-1, 0-2, 0-3 is cut, no recoding needed for this case
  { 45, 3, 1, 2, 0, 12 }, /// case 4a: edge 0-1, 0-2, 0-3, 2-3 is cut, this just an additional subcase to avoid negative volumes (otherwise identical with case 4a)
  { 46, 3, 2, 1, 0, 7 }, /// case 4a: edge 0-1, 0-2, 0-3, 1-3 is cut
  { 47, 1, 2, 0, 3, 9 }, /// case  5: edge 0-1, 0-2, 0-3, 1-3, 2-3 is cut
  { 48, 0, 1, 2, 3, 2 }, /// case 2a: edge 0-1, 1-2 is cut, no recoding needed for this case
  { 49, 2, 1, 0, 3, 11 }, /// case 3c: edge 0-1, 1-2, 2-3 is cut, this just an additional subcase to avoid negative volumes (otherwise identical with case 3c)
  { 50, 1, 3, 2, 0, 4 }, /// case 3a: edge 0-1, 1-2, 1-3 is cut, HERE
  { 51, 3, 0, 2, 1, 7 }, /// case 4a: edge 0-1, 1-2, 1-3, 2-3 is cut
  { 52, 0, 1, 2, 3, 6 }, /// case 3c: edge 0-1, 0-3, 1-2 is cut, no recoding needed for this case
  { 53, 1, 3, 2, 0, 8 }, /// case 4b: edge 0-1, 0-3, 1-2, 2-3 is cut
  { 54, 0, 2, 3, 1, 7 }, /// case 4a: edge 0-1, 0-3, 1-2, 1-3 is cut
  { 55, 0, 2, 3, 1, 9 }, /// case  5: edge 0-1, 0-3, 1-2, 1-3, 2-3 is cut
  { 56, 0, 2, 3, 1, 5 }, /// case 3b: edge 0-1, 0-2, 1-2 is cut
  { 57, 0, 3, 1, 2, 7 }, /// case 4a: edge 0-1, 0-2, 1-2, 2-3 is cut
  { 58, 2, 3, 0, 1, 7 }, /// case 4a: edge 0-1, 0-2, 1-2, 1-3 is cut
  { 59, 3, 0, 2, 1, 9 }, /// case  5: edge 0-1, 0-2, 1-2, 1-3, 2-3 is cut
  { 60, 1, 3, 2, 0, 7 }, /// case 4a: edge 0-1, 0-2, 0-3, 1-2 is cut
  { 61, 1, 3, 2, 0, 9 }, /// case  5: edge 0-1, 0-2, 0-3, 1-2, 2-3 is cut
  { 62, 3, 2, 1, 0, 9 }, /// case  5: edge 0-1, 0-2, 0-3, 1-2, 1-3 is cut
  { 63, 0, 1, 2, 3, 10 } /// case  6: edge 0-1, 0-2, 0-3, 1-2, 1-3, 2-3, no recoded needed for this case
};

RefineTetMeshLocallyAlgorithm::RefineTetMeshLocallyAlgorithm()
{
  using namespace Parameters;

  addParameter(RefineTetMeshLocallyIsoValue, 0.0);
  addParameter(RefineTetMeshLocallyEdgeLength, 0.0);
  addParameter(RefineTetMeshLocallyVolume, 0.0);
  addParameter(RefineTetMeshLocallyDihedralAngleSmaller, 180.0);
  addParameter(RefineTetMeshLocallyDihedralAngleBigger, 0.0);
  addParameter(RefineTetMeshLocallyRadioButtons, 0);
  addParameter(RefineTetMeshLocallyDoNoSplitSurfaceTets, false);
  addParameter(RefineTetMeshLocallyCounterClockWiseOrdering, false);
  addParameter(RefineTetMeshLocallyUseModuleInputField, false);
  addParameter(RefineTetMeshLocallyMaxNumberRefinementIterations, 5);
}

AlgorithmOutput RefineTetMeshLocallyAlgorithm::run(const AlgorithmInput& input) const
{
  auto inputfield = input.get<Field>(Variables::InputField);
  //auto inputmatrix = input.get<DenseMatrix>(Variables::InputMatrix);
  FieldHandle outputField;

  if (!runImpl(inputfield, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;

  return output;
}

std::vector<int> RefineTetMeshLocallyAlgorithm::SelectMeshElements(FieldHandle input, int choose_refinement_option, int& count) const
{
  /// if a matrix second module input is provided used for element selection (that get split) and ignore GUI settings
  std::vector<int> result;
  using namespace Parameters;

  VMesh* input_vmesh = input->vmesh();
  VField* input_vfld = input->vfield();
  input_vmesh->synchronize(Mesh::NODES_E);

  bool ModuleInput = get(RefineTetMeshLocallyUseModuleInputField).toBool();
  result.resize(input_vfld->num_values());

  if (ModuleInput || choose_refinement_option == 0) /// this is the isovalue selection criteria, it also covers the preselection from module input
  {
    double value = get(RefineTetMeshLocallyIsoValue).toDouble();

    if (ModuleInput && value != code_to_split && choose_refinement_option == 0)
    {
      /// if some tets are preselected in module input (have field data value "1") but 'Iso Value' is chosen NOT "1" (with active radiobutton on "Iso Value"), this module will not
      /// preselection works as a logical AND in order to be able to be more specific what to refine
    }
    else
    {
      if (ModuleInput)
        value = code_to_split;

      for (VMesh::Elem::index_type idx = 0; idx < input_vmesh->num_elems(); ++idx)
      {
        double tmp;
        input_vfld->get_value(tmp, idx);
        if (tmp == value)
        {
          result[static_cast<int>(idx)] = code_to_split;
          count++;
        }
        else
        {
          result[static_cast<int>(idx)] = code_not_to_split;
        }
      }
    }
  }

  switch (choose_refinement_option)
  {
  case 1: /// this is the edge selection criteria
  {
    double value = get(RefineTetMeshLocallyEdgeLength).toDouble();
    std::vector<double> edge_lengths(6);
    std::vector<int> pos;

    for (VMesh::Elem::index_type idx = 0; idx < input_vmesh->num_elems(); ++idx)
    {
      VMesh::Node::array_type onodes(4);
      input_vmesh->get_nodes(onodes, idx);
      Point p1, p2, p3, p4;
      input_vmesh->get_center(p1, onodes[0]);
      input_vmesh->get_center(p2, onodes[1]);
      input_vmesh->get_center(p3, onodes[2]);
      input_vmesh->get_center(p4, onodes[3]);
      edge_lengths = getEdgeLengths(p1, p2, p3, p4);
      pos = maxi(edge_lengths);

      bool condition = edge_lengths[pos[0]] > value;
      bool ext_condition = result[static_cast<long>(idx)] && ModuleInput;
      if (condition && !ModuleInput) /// typical case
      {
        result[static_cast<long>(idx)] = code_to_split;
        count++;
      }
      else
        if (!condition && ext_condition)
        {
          result[static_cast<long>(idx)] = code_not_to_split;
          count--;
        }
        else
          if (condition && ext_condition)
          {
            ///its already selected no need to do that again
          }
          else
          {
            result[static_cast<long>(idx)] = code_not_to_split;
          }
    }

    break;

  }
  case 2: /// this is the volume selection criteria
  {
    double volume_bound = get(RefineTetMeshLocallyVolume).toDouble();

    for (VMesh::Elem::index_type idx = 0; idx < input_vmesh->num_elems(); ++idx)
    {
      VMesh::Node::array_type onodes(4);
      input_vmesh->get_nodes(onodes, idx);
      Point p1, p2, p3, p4;
      input_vmesh->get_center(p1, onodes[0]);
      input_vmesh->get_center(p2, onodes[1]);
      input_vmesh->get_center(p3, onodes[2]);
      input_vmesh->get_center(p4, onodes[3]);
      double x1 = p1.x(), y1 = p1.y(), z1 = p1.z(),
        x2 = p2.x(), y2 = p2.y(), z2 = p2.z(),
        x3 = p3.x(), y3 = p3.y(), z3 = p3.z(),
        x4 = p4.x(), y4 = p4.y(), z4 = p4.z();

      double tet_volume = (x3*(y2*z1 - y1*z2) + x2*(y1*z3 - y3*z1) - x1*(y2*z3 - y3*z2)) + (-x4*(y2*z1 - y1*z2) - x2* (y1*z4 - y4*z1) + x1* (y2*z4 - y4*z2)) - (-x4*(y3*z1 - y1*z3) - x3*(y1*z4 - y4*z1) + x1*(y3*z4 - y4*z3)) + (-x4* (y3*z2 - y2*z3) - x3* (y2*z4 - y4*z2) + x2* (y3*z4 - y4*z3));
      tet_volume /= 6;

      if (tet_volume <= 0)
      {
        remark(" The volume of at least one mesh element is zero or even negative. If its negative you can use 'counterclockwise tet ordering'. If its zero the tet might be flat ");
        //return result;
      }

      bool condition = tet_volume > volume_bound;
      bool ext_condition = result[static_cast<long>(idx)] && ModuleInput;
      if (condition && !ModuleInput) /// typical case
      {
        result[static_cast<long>(idx)] = code_to_split;
        count++;
      }
      else
        if (!condition && ext_condition)
        {
          result[static_cast<long>(idx)] = code_not_to_split;
          count--;
        }
        else
          if (condition && ext_condition)
          {
            ///its already selected no need to do that again
          }
          else
          {
            result[static_cast<long>(idx)] = code_not_to_split;
          }

    }
    break;
  }
  case 3:
  {
    std::vector<Point> points(3);
    VMesh::Node::array_type nodes;
    VMesh::Face::array_type faces;
    double min, max;
    input_vmesh->synchronize(Mesh::NODES_E | Mesh::FACES_E);
    double min_bound = get(RefineTetMeshLocallyDihedralAngleBigger).toDouble();
    double max_bound = get(RefineTetMeshLocallyDihedralAngleSmaller).toDouble();

    for (VMesh::Elem::index_type idx = 0; idx < input_vmesh->num_elems(); ++idx)
    {
      input_vmesh->get_faces(faces, idx);
      if (faces.size() != 4)
      {
        error(" The chosen refinement criteria is not valid. This message should not appear!!!");
        std::vector<int> tmp;
        return tmp;
      }

      min = std::numeric_limits<double>::max(); max = std::numeric_limits<double>::min();
      for (int j = 0; j < number_faces; j++)
      {
        for (int k = j + 1; k < number_faces; k++)
        {
          input_vmesh->get_nodes(nodes, faces[j]);
          input_vmesh->get_centers(points, nodes);
          Vector normal1 = Cross(points[1] - points[0], points[2] - points[0]);
          normal1.safe_normalize();
          input_vmesh->get_nodes(nodes, faces[k]);
          input_vmesh->get_centers(points, nodes);
          Vector normal2 = Cross(points[1] - points[0], points[2] - points[0]);
          normal2.safe_normalize();
          double dot_product = Dot(normal1, normal2);
          if (dot_product < -1)
          {
            dot_product = -1;
          }
          else if (dot_product > 1)
          {
            dot_product = 1;
          }
          double dihedral_angle = 180.0 - acos(dot_product) * 180.0 / M_PI;

          if (dihedral_angle < min)
          {
            min = dihedral_angle;
          }
          else if (dihedral_angle > max)
          {
            max = dihedral_angle;
          }
        }
      }

      bool condition = min >= min_bound && max <= max_bound;
      bool ext_condition = result[static_cast<long>(idx)] && ModuleInput;
      if (condition && !ModuleInput) /// typical case
      {
        result[static_cast<long>(idx)] = code_to_split;
        count++;
      }
      else
        if (!condition && ext_condition)
        {
          result[static_cast<long>(idx)] = code_not_to_split;
          count--;
        }
        else
          if (condition && ext_condition)
          {
            ///its already selected no need to do that again
          }
          else
          {
            result[static_cast<long>(idx)] = code_not_to_split;
          }

    }
    break;
  }
  default:
  {
    if (choose_refinement_option < 0 || choose_refinement_option>3)
    {
      error("Internal error: refinement option is not implemented.");
      std::vector<int> tmp;
      return tmp;
    }
    break;
  }
  }

  if (input_vmesh->num_elems() != result.size())
  {
    error(" Size of selection vector is unexpected ");
    std::vector<int> tmp;
    return tmp;
  }

  return result;
}

std::vector<double> RefineTetMeshLocallyAlgorithm::getEdgeLengths(Point p1, Point p2, Point p3, Point p4) const
{
  std::vector<double> edge_lengths(6);

  double x1 = p1.x(), y1 = p1.y(), z1 = p1.z(),
    x2 = p2.x(), y2 = p2.y(), z2 = p2.z(),
    x3 = p3.x(), y3 = p3.y(), z3 = p3.z(),
    x4 = p4.x(), y4 = p4.y(), z4 = p4.z();
  double e12 = sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2) + (z1 - z2)*(z1 - z2)),
    e14 = sqrt((x1 - x4)*(x1 - x4) + (y1 - y4)*(y1 - y4) + (z1 - z4)*(z1 - z4)),
    e13 = sqrt((x1 - x3)*(x1 - x3) + (y1 - y3)*(y1 - y3) + (z1 - z3)*(z1 - z3)),
    e34 = sqrt((x3 - x4)*(x3 - x4) + (y3 - y4)*(y3 - y4) + (z3 - z4)*(z3 - z4)),
    e23 = sqrt((x2 - x3)*(x2 - x3) + (y2 - y3)*(y2 - y3) + (z2 - z3)*(z2 - z3)),
    e24 = sqrt((x2 - x4)*(x2 - x4) + (y2 - y4)*(y2 - y4) + (z2 - z4)*(z2 - z4));
  edge_lengths[0] = e12; edge_lengths[1] = e23; edge_lengths[2] = e13; edge_lengths[3] = e14; edge_lengths[4] = e24; edge_lengths[5] = e34;

  return edge_lengths;
}

std::vector<int> RefineTetMeshLocallyAlgorithm::getEdgeCoding(int pos) const
{
  std::vector<int> edgecoding;
  edgecoding.resize(2);
  switch (pos)
  {
  default:
  {
    error("Error in getEdgeCoding. This message should never appear. ");
    return edgecoding;
  }
  case 0: // 0-1
  {
    edgecoding[0] = 1;
    edgecoding[1] = 2;
    break;
  }
  case 1: // 1-2
  {
    edgecoding[0] = 2;
    edgecoding[1] = 3;
    break;
  }
  case 2: // 0-2
  {
    edgecoding[0] = 1;
    edgecoding[1] = 3;
    break;
  }
  case 3: // 0-3
  {
    edgecoding[0] = 1;
    edgecoding[1] = 4;
    break;
  }
  case 4: // 1-3
  {
    edgecoding[0] = 2;
    edgecoding[1] = 4;
    break;
  }
  case 5: // 2-3
  {
    edgecoding[0] = 3;
    edgecoding[1] = 4;
    break;
  }
  }
  return edgecoding;
}

std::vector<int> RefineTetMeshLocallyAlgorithm::maxi(const std::vector<double>& input_vec) const
{
  std::vector<int> result(6);
  double maximum = std::numeric_limits<double>::min();

  if (input_vec.size() != number_edges)
  {
    error(" RefineTetMeshLocallyAlgorithm::maxi input error ");
    return result;
  }

  for (int i = 0; i < number_edges; i++)
  {
    if (maximum < input_vec[i])
    {
      maximum = input_vec[i];
    }
  }

  int count = 0;
  for (int i = 0; i < number_edges; i++)
  {
    if (maximum == input_vec[i])
    {
      result[count++] = i;
    }
  }

  result.resize(count);

  return result;
}

/// TODO (DAN): this function should run in parallel
SparseRowMatrixHandle RefineTetMeshLocallyAlgorithm::ChoseEdgesToCut(FieldHandle input, const std::vector<long>& elems_to_split, VMesh* field_boundary) const
{
  VMesh* input_vmesh = input->vmesh();
  size_t number = elems_to_split.size();
  SparseRowMatrixHandle cut_edges;
  std::vector<double> edge_lengths;
  SparseRowMatrixFromMap::Values cut_edges_val; /// all tet element counting starts from 1 (0 denotes blank)

  VMesh::Node::array_type onodes(4);
  Point p1, p2, p3, p4;
  input_vmesh->synchronize(Mesh::NODES_E);

  bool split_it = true;
  bool MeshDoNoSplitSurfaceTets = get(Parameters::RefineTetMeshLocallyDoNoSplitSurfaceTets).toBool();

  for (long idx = 0; idx < number; idx++) // this for loop can be parallelized
  {
    input_vmesh->get_nodes(onodes, static_cast<VMesh::Elem::index_type>(elems_to_split[idx]));
    input_vmesh->get_center(p1, static_cast<VMesh::Node::index_type>(onodes[0]));
    input_vmesh->get_center(p2, static_cast<VMesh::Node::index_type>(onodes[1]));
    input_vmesh->get_center(p3, static_cast<VMesh::Node::index_type>(onodes[2]));
    input_vmesh->get_center(p4, static_cast<VMesh::Node::index_type>(onodes[3]));

    if (MeshDoNoSplitSurfaceTets)
    {
      double dis1 = std::numeric_limits<double>::max(), dis2 = std::numeric_limits<double>::max(),
        dis3 = std::numeric_limits<double>::max(), dis4 = std::numeric_limits<double>::max();
      VMesh::Node::index_type didx;
      Point r;
      field_boundary->find_closest_node(dis1, r, didx, p1);
      field_boundary->find_closest_node(dis2, r, didx, p2);
      field_boundary->find_closest_node(dis3, r, didx, p3);
      field_boundary->find_closest_node(dis4, r, didx, p4);
      if (dis1 == 0 || dis2 == 0 || dis3 == 0 || dis4 == 0)
      {
        split_it = false;
      }
    }

    if (split_it)
    {
      edge_lengths = getEdgeLengths(p1, p2, p3, p4);
      std::vector<int> pos = maxi(edge_lengths);

      if (pos.size() == 0)
      {
        error("The function RefineTetMeshLocallyAlgorith::maxi(), that determines the number of cut points. Its 0, this should never happen. ");
        return cut_edges;
      }

      for (int j = 0; j < pos.size(); j++)
      {
        std::vector<int> edgecode = getEdgeCoding(pos[j]);
        long e1 = onodes[edgecode[0] - 1], e2 = onodes[edgecode[1] - 1];
        cut_edges_val[e1 < e2 ? e1 : e2][e2 >= e1 ? e2 : e1] = 1;
      }
    }
    split_it = true;
  }

  cut_edges = SparseRowMatrixFromMap::make(input_vmesh->num_nodes(), input_vmesh->num_nodes(), cut_edges_val);
  return cut_edges;
}

/// TODO (DAN): this function should run in parallel too
FieldHandle RefineTetMeshLocallyAlgorithm::RefineMesh(FieldHandle input, SparseRowMatrixHandle cut_edges) const
{
  FieldHandle output, result;
  double fld_val;
  DenseMatrixHandle two_new_tets(new DenseMatrix(2, 4));
  VMesh* input_vmesh = input->vmesh();
  VField* input_vfield = input->vfield();
  input_vmesh->synchronize(Mesh::NODES_E);
  long number_elem = input_vmesh->num_elems(), node_count = input_vmesh->num_nodes();
  VMesh::Node::array_type onodes(4), onodes2(4);

  if (cut_edges->nrows() != node_count)
  {
    error(" RefinedMesh() inputs inconsistent. ");
    return output;
  }

  if (cut_edges->ncols() != node_count)
  {
    error(" RefinedMesh() inputs inconsistent. ");
    return output;
  }

  FieldInformation fieldinfo("TetVolMesh", 0, "double");
  result = CreateField(fieldinfo);
  VMesh* result_vmesh = result->vmesh();
  VField* result_vfld = result->vfield();

  ///count how many tets and nodes are needed now
  long tet_count = 0;

  Point p1, p2, p3, p4;

  for (long idx = 0; idx < node_count; idx++)
  {
    input_vmesh->get_center(p1, static_cast<VMesh::Node::index_type>(idx));
    result_vmesh->add_point(Point(p1.x(), p1.y(), p1.z()));
  }

  for (long idx = 0; idx < number_elem; idx++)
  {
    input_vfield->get_value(fld_val, idx);
    input_vmesh->get_nodes(onodes, static_cast<VMesh::Elem::index_type>(idx));

    int main_case, case_code = 0, nr_tets;
    int recode[4];
    for (int k = 0; k < number_edges; k++)
    {
      long e1 = onodes[EdgeLookup[k][1]], e2 = onodes[EdgeLookup[k][2]];

      if ((*cut_edges).coeff(e1 < e2 ? e1 : e2, e2 >= e1 ? e2 : e1) == 1)
      {
        case_code += pow(2.0, 9 - (EdgeLookup[k][0]));
      }
    }

    /// no cutting edge was found but that should actually not happen
    if (case_code<0 || case_code>number_cases - 1)
    {
      std::ostringstream ostr;
      ostr << " Case " << case_code << " is not specified (range: 1.." << number_cases << ") in the code and therefore appears to be broken. " << std::endl;
      error(ostr.str());
      break;
    }

    if (case_code != 0) /// start refining here
    {

      for (int k = 0; k < number_nodes; k++)
        recode[k] = CaseLookup[case_code - 1][k + 1];

      main_case = CaseLookup[case_code - 1][5];
      /// out of 64 (2^6) theoretical cases to split a tetrahedron, there are only 10 that are actually relavant because of symmetry

      nr_tets = NumberTets[main_case - 1];

      for (int k = 0; k < nr_tets; k++)
      {
        for (int l = 0; l < number_nodes; l++)
        {
          int node = 0;

          if (main_case == 1)
            node = Case1Lookup[k][l]; else
            if (main_case == 2)
              node = Case2aLookup[k][l]; else
              if (main_case == 3)
                node = Case2bLookup[k][l]; else
                if (main_case == 4)
                  node = Case3aLookup[k][l]; else
                  if (main_case == 5)
                    node = Case3bLookup[k][l]; else
                    if (main_case == 6)
                      node = Case3cLookup[k][l]; else
                      if (main_case == 7)
                        node = Case4aLookup[k][l]; else
                        if (main_case == 8)
                          node = Case4bLookup[k][l]; else
                          if (main_case == 9)
                            node = Case5Lookup[k][l]; else
                            if (main_case == 10)
                              node = Case6Lookup[k][l]; else
                              if (main_case == 11)
                                node = Case3cNonNegativeLookup[k][l]; else
                                if (main_case == 12)
                                  node = Case4aNonNegativeLookup[k][l];

          if (node > 3 && node <= 9)
          {
            input_vmesh->get_center(p1, static_cast<VMesh::Node::index_type>(onodes[recode[EdgeLookup[node - 4][1]]]));
            input_vmesh->get_center(p2, static_cast<VMesh::Node::index_type>(onodes[recode[EdgeLookup[node - 4][2]]]));
            onodes2[l] = node_count;
            result_vmesh->add_point(Point((p1.x() + p2.x()) / 2, (p1.y() + p2.y()) / 2, (p1.z() + p2.z()) / 2));
            node_count++;
          }
          else
            if (node == 10)
              /// this is an addition to the splitting algorithm proposed in Thompson, all edges of the new tets should have smaller edges (in case every edge of the
              /// original tet needs to be split)
            {
              input_vmesh->get_center(p1, static_cast<VMesh::Node::index_type>(onodes[0]));
              input_vmesh->get_center(p2, static_cast<VMesh::Node::index_type>(onodes[1]));
              input_vmesh->get_center(p3, static_cast<VMesh::Node::index_type>(onodes[2]));
              input_vmesh->get_center(p4, static_cast<VMesh::Node::index_type>(onodes[3]));
              onodes2[l] = node_count;
              result_vmesh->add_point(Point((p1.x() + p2.x() + p3.x() + p4.x()) / 4, (p1.y() + p2.y() + p3.y() + p4.y()) / 4, (p1.z() + p2.z() + p3.z() + p4.z()) / 4));
              node_count++;
            }
            else
            {
              onodes2[l] = onodes[recode[node]];
            }
        }
        result_vmesh->add_elem(onodes2);
        result_vfld->resize_values();
        result_vfld->set_value(fld_val, tet_count++);
      }
    }
    else
    { /// current tet is not selected to be cut -> just add it as it is to the output mesh
      result_vmesh->add_elem(onodes);
      result_vfld->resize_values();
      result_vfld->set_value(fld_val, tet_count++);
    }
  }

  JoinFieldsAlgo joinfields_algo; /// use joinfields to get rid of extra nodes and duplicated elements

  joinfields_algo.set(JoinFieldsAlgo::MergeNodes, true);
  joinfields_algo.set(JoinFieldsAlgo::MergeElems, true);
  FieldList input_list; /// need input_list only for joinfields
  input_list.push_back(result);
  input_list.push_back(result);
  joinfields_algo.runImpl(input_list, output);

  return output;
}

bool RefineTetMeshLocallyAlgorithm::runImpl(FieldHandle input, FieldHandle& output) const
{
  if (!input)
  {
    error("First module input (Field) is empty.");
    return false;
  }

  FieldInformation fi(input);

  if (!(fi.is_tetvolmesh()))
  {
    error("The input field data needs to be a tetvolmesh generated by the cleaver meshing package.");
    return false;
  }

  if (!(fi.is_constantdata()))
  {
    error("The input field data (first module input) need to be located at the elements");
    return false;
  }

  using namespace Parameters;
  int RadioButtonChoice = get(RefineTetMeshLocallyRadioButtons).toInt();
  int NumberOfIterations = get(RefineTetMeshLocallyMaxNumberRefinementIterations).toInt();
  bool invert_tet_ordering = get(RefineTetMeshLocallyCounterClockWiseOrdering).toBool();
  std::vector<int> selection_vector;

  int count = 0, total_iterations = 0;

  bool MeshDoNoSplitSurfaceTets = get(RefineTetMeshLocallyDoNoSplitSurfaceTets).toBool();
  GetFieldBoundaryAlgo getfieldbound_algo;
  MatrixHandle mapping;
  FieldHandle field_boundary;
  VMesh* field_boundry_vmesh = nullptr;

  if (MeshDoNoSplitSurfaceTets)
  {
    getfieldbound_algo.run(input, field_boundary, mapping);
    field_boundry_vmesh = field_boundary->vmesh();
    field_boundry_vmesh->synchronize(Mesh::FIND_CLOSEST_NODE_E);
  }

  output = input;

  for (int j = 0; j < NumberOfIterations; j++)
  {
    count = 0;
    selection_vector = SelectMeshElements(output, RadioButtonChoice, count);

    if (selection_vector.size() == 0 || count <= 0)
    {
      std::ostringstream ostr;
      ostr << " No (more) tetrahedral elements need to be split. " << std::endl;
      remark(ostr.str());
      break;
    }

    total_iterations++;

    if (output->vmesh()->num_elems() != selection_vector.size())
    {
      std::ostringstream ostr;
      ostr << " Internal error: length of selection vector is not equal to number of input mesh elements. " << std::endl;
      error(ostr.str());
      return true;
    }

    std::vector<long> elems_to_split;
    elems_to_split.resize(count);

    if (count > 0)
    {
      if (invert_tet_ordering && j == 0)
      {
        for (int i = 0; i < number_cases; i++)
        {
          count = CaseLookup[i][1];
          CaseLookup[i][1] = CaseLookup[i][2];
          CaseLookup[i][2] = count;
        }
      }

      count = 0;
      for (long i = 0; i < selection_vector.size(); i++)
      {
        if (selection_vector[i] == code_to_split)
        {
          elems_to_split[count++] = i;
        }
      }

      auto cut_edges = ChoseEdgesToCut(output, elems_to_split, field_boundry_vmesh);
      if (cut_edges->nonZeros() == 0)
        break;

      output = RefineMesh(output, cut_edges);

    }
  }

  if (total_iterations > 0)
  {
    std::ostringstream ostr;
    ostr << total_iterations << " Iterations until selection criteria was reached ! " << std::endl;
    remark(ostr.str());
  }

  return (true);
}
