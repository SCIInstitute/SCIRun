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


///@file GenerateROIStatisticsAlgorithm
///@brief The algorithm implements a statistical ROI analysis.
///
///@author
/// Moritz Dannhauer
///
///@details
/// The algorithm provides results for all ROIs (if radius in lower table is 0) specified as label numbers in the atlas mesh (second module input). Based on a location (x,y,z)
/// provided in the lower GUI table the user can specify an own ROI. The algorithm uses this (x,y,z) location to determine the closest mesh element based on the element centers.
/// A spherical ROI can be wrapped around that point (closest mesh element center) and an statistical analysis of all material in that sphere can be performed.
/// If additionally, a material (lower GUI table, in Atlas Material #) was specified only the analysis is only performed for the specified material.
/// In any case, if an material was specified but not found the algorithm outputs NaN. "0" or " "  as Atlas Material # is interpreted as any material label number.
/// If one material was present only as a single element the standard deviation (stddev.)

#ifndef ALGORITHMS_MATH_GenerateROIStatisticsAlgorithm_H
#define ALGORITHMS_MATH_GenerateROIStatisticsAlgorithm_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/BrainStimulator/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace BrainStimulator {

  ALGORITHM_PARAMETER_DECL(ROITableValues);
  ALGORITHM_PARAMETER_DECL(StatisticsTableValues);
  ALGORITHM_PARAMETER_DECL(PhysicalUnitStr);
  ALGORITHM_PARAMETER_DECL(CoordinateSpaceLabelStr);

  class SCISHARE GenerateROIStatisticsAlgorithm : public AlgorithmBase
  {
  public:
    AlgorithmOutput run(const AlgorithmInput& input) const override;
    GenerateROIStatisticsAlgorithm();
    static const AlgorithmInputName MeshDataOnElements;
    static const AlgorithmInputName PhysicalUnit;
    static const AlgorithmInputName AtlasMesh;
    static const AlgorithmInputName AtlasMeshLabels;
    static const AlgorithmInputName CoordinateSpace;
    static const AlgorithmInputName CoordinateSpaceLabel;
    static const AlgorithmInputName SpecifyROI;
    static const AlgorithmOutputName StatisticalResults;
    boost::tuple<Datatypes::DenseMatrixHandle, VariableHandle> run(FieldHandle mesh, FieldHandle AtlasMesh, const FieldHandle CoordinateSpace=FieldHandle(), const std::string& AtlasMeshLabels="", const Datatypes::DenseMatrixHandle specROI=Datatypes::DenseMatrixHandle()) const;

  private:
    std::vector<std::string> ConvertInputAtlasStringIntoVector(const  std::string& atlasLabels) const;
    std::vector<bool> statistics_based_on_xyz_coodinates(const FieldHandle mesh, const FieldHandle CoordinateSpace, double x, double y, double z, double radius, int target_material) const;
  };

}}}}

#endif
