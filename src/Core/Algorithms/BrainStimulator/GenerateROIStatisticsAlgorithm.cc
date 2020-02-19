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


#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/BrainStimulator/GenerateROIStatisticsAlgorithm.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/String.h>
#include <boost/range/algorithm/count.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <Core/Logging/Log.h>
#include <string>
#include <iostream>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;
using namespace SCIRun::Core::Logging;
using namespace boost::assign;

const AlgorithmInputName GenerateROIStatisticsAlgorithm::MeshDataOnElements("MeshDataOnElements");
const AlgorithmInputName GenerateROIStatisticsAlgorithm::PhysicalUnit("PhysicalUnit");
const AlgorithmInputName GenerateROIStatisticsAlgorithm::AtlasMesh("AtlasMesh");
const AlgorithmInputName GenerateROIStatisticsAlgorithm::AtlasMeshLabels("AtlasMeshLabels");
const AlgorithmInputName GenerateROIStatisticsAlgorithm::CoordinateSpace("CoordinateSpace");
const AlgorithmInputName GenerateROIStatisticsAlgorithm::CoordinateSpaceLabel("CoordinateSpaceLabel");
const AlgorithmInputName GenerateROIStatisticsAlgorithm::SpecifyROI("SpecifyROI");
const AlgorithmOutputName GenerateROIStatisticsAlgorithm::StatisticalResults("StatisticalResults");

ALGORITHM_PARAMETER_DEF(BrainStimulator, ROITableValues);
ALGORITHM_PARAMETER_DEF(BrainStimulator, StatisticsTableValues);
ALGORITHM_PARAMETER_DEF(BrainStimulator, PhysicalUnitStr);
ALGORITHM_PARAMETER_DEF(BrainStimulator, CoordinateSpaceLabelStr);

GenerateROIStatisticsAlgorithm::GenerateROIStatisticsAlgorithm()
{
  using namespace Parameters;
  addParameter(ROITableValues, 0);
  addParameter(StatisticsTableValues, 0);
  addParameter(PhysicalUnitStr, std::string());
  addParameter(CoordinateSpaceLabelStr, std::string());
}

namespace
{
  std::string formatStatistic(double x)
  {
    if (IsNan(x))
      return "NaN";
    return boost::str(boost::format("%.3f") % x);
  }

  std::string formatCount(double x)
  {
    if (IsNan(x))
      return "NaN";
    return boost::str(boost::format("%d") % x);
  }
}

/// the run function can deal with multiple inputs and performs the analysis for all ROIs in the atlas mesh and for the user specified ROI
boost::tuple<DenseMatrixHandle, VariableHandle> GenerateROIStatisticsAlgorithm::run(FieldHandle mesh, FieldHandle AtlasMesh, const FieldHandle CoordinateSpace, const std::string& AtlasMeshLabels, const DenseMatrixHandle specROI) const
{
  VField* vfield1 = mesh->vfield();
  VField* vfield2 = AtlasMesh->vfield();

  std::vector<bool> element_selection(vfield2->vmesh()->num_elems(), true);  /// set the default element ROI selection, so all of them are included

  double x=0,y=0,z=0,radius=-1,target_material=-1;
  /// CoordinateSpace is provided and coordinates? if not don't go in this if clause
  if (CoordinateSpace != nullptr && specROI != nullptr)
  {
    if ( (*specROI).ncols()==1 && (*specROI).nrows()==5 ) /// GUI input (as DenseMatrix) has the right sizes (rows, cols)?
    {
      x=(*specROI)(0,0);
      y=(*specROI)(1,0);
      z=(*specROI)(2,0);
      target_material=(*specROI)(3,0);
      radius=(*specROI)(4,0);

      if (radius<0) /// if radius < 0, its invalid input provide a remark and do the statistical analysis for all ROIs instead
      {
        remark("Radius needs to be > 0 and Atlas Material # needs to exist ");
      } else
      {
        if (radius>0)
        {
          element_selection = statistics_based_on_xyz_coodinates(AtlasMesh, CoordinateSpace, x, y, z, radius, target_material); /// redefine the element ROI selection if correct input was delivered
        }
      }
    }

  }

  if(element_selection.size()!=vfield1->vmesh()->num_elems()) /// internal error check if selection vector really matches number of mesh elements
  {
    THROW_ALGORITHM_INPUT_ERROR("Internal Error: Element selection vector does not match number of mesh elements ");
  }

  size_t number_of_atlas_materials;

  std::set<int> labelSet;

  if (target_material==-1 || radius==0) /// if default consider all materials
  {
    for (VMesh::Elem::index_type i=0; i < vfield2->vmesh()->num_elems(); i++) // loop over all tetrahedral elements (mesh)
    {
      int Label;
      vfield2->get_value(Label, i);
      labelSet.insert(Label);
    }
  } else
  {
    labelSet.insert(static_cast<int>(target_material));
  }

  number_of_atlas_materials = labelSet.size();

  std::vector<int> labelVector(labelSet.begin(), labelSet.end());

  std::ostringstream ostr; /// sort element labels ascending
  std::copy(labelSet.begin(), labelSet.end(), std::ostream_iterator<int>(ostr, ", "));
  LOG_DEBUG("Sorted set of label numbers: {}", ostr.str());

  std::vector<double> value_avr(number_of_atlas_materials);
  std::vector<int> value_count(number_of_atlas_materials);
  std::vector<double> value_min(number_of_atlas_materials);
  std::vector<double> value_max(number_of_atlas_materials);
  std::vector<double> value_std(number_of_atlas_materials);
  std::vector<double> Sxsqr(number_of_atlas_materials);
  std::vector<double> stddev(number_of_atlas_materials);
  std::vector<double> var(number_of_atlas_materials);

  /// to do the actual statistics we need to precompute parts of it to make it efficient (don't loop over elements twice)
  for (VMesh::Elem::index_type i=0; i < vfield1->vmesh()->num_elems(); i++) /// loop over all tetrahedral elements (AtlasMesh)
  {

    if(element_selection[i]) ///is an particular element selected?
    {
      double value = 0;
      vfield1->get_value(value, i);

      int Label = 0;
      vfield2->get_value(Label, i);

      for (VMesh::Elem::index_type j=0; j < number_of_atlas_materials; ++j) /// loop over determined materials
      {
        if (Label==labelVector[j] || (target_material==0 && number_of_atlas_materials==1) ) /// if label is known or if default situation precalculate sum, min, max, sum^2, number of selected elements
        {
          value_avr[j] += value;
          value_count[j]++;
          if (value>value_max[j]) value_max[j]=value;
          if (value<value_min[j] || value_min[j]==0) value_min[j]=value;
          Sxsqr[j]+=value*value;
        }
      }
    }
  }

  DenseMatrixHandle output(new DenseMatrix(number_of_atlas_materials, 5));
  const double invalidDouble = std::numeric_limits<double>::quiet_NaN();

  /// efficient way to compute std dev. in just one loop over all mesh elements: sqrt ( 1/(n-1) (Sx^2 - avr Sx + n avr^2 )
  for (VMesh::Elem::index_type j=0; j < number_of_atlas_materials; ++j)
  {
    double Sx=value_avr[j];

    if (value_count[j]!=0)
    {
      value_avr[j]/=value_count[j];
      if (value_count[j]>1)
      {
        var[j]=static_cast<double>(1./(value_count[j]-1)*(Sxsqr[j]-2*value_avr[j]*Sx+value_count[j]*value_avr[j]*value_avr[j]));
        stddev[j]=static_cast<double>(std::sqrt(var[j])); /// compute standard deviation, average, variance
      } else
      {
        var[j]=invalidDouble;
        stddev[j]=invalidDouble;
      }

      (*output)(j,0)=value_avr[j]; /// save statistical measures in output (DenseMatrix)
      (*output)(j,1)=stddev[j];
      (*output)(j,2)=value_min[j];
      (*output)(j,3)=value_max[j];
      (*output)(j,4)=value_count[j];
    } else
    {
      (*output)(j,0)=invalidDouble;  /// if the number of elements is 0, provide NaN as output
      (*output)(j,1)=invalidDouble;
      (*output)(j,2)=invalidDouble;
      (*output)(j,3)=invalidDouble;
      (*output)(j,4)=invalidDouble;
    }
  }

  std::vector<std::string> AtlasMeshLabels_vector;
  if (!AtlasMeshLabels.empty())
  {
    AtlasMeshLabels_vector = ConvertInputAtlasStringIntoVector(AtlasMeshLabels); /// cut the atlas ROI labels into pieces (std::vector)
  }

  if (AtlasMeshLabels_vector.empty() && number_of_atlas_materials>0)
  {
    AtlasMeshLabels_vector.resize(number_of_atlas_materials);
    for (int i=0; i<number_of_atlas_materials; i++)
    {
      AtlasMeshLabels_vector[i]=boost::lexical_cast<std::string>(i+1); /// if no atlas ROI labels are provided use consecutive numbers
    }
  }
  else
    if (AtlasMeshLabels_vector.size() != number_of_atlas_materials)
    {
      if (target_material!=-1 && number_of_atlas_materials==1)
      {
        AtlasMeshLabels_vector.resize(1);
        AtlasMeshLabels_vector[0]="specROI";  /// if an ROI was specified by the user use specROI label for the upper table
      }
      else
      {
        THROW_ALGORITHM_INPUT_ERROR("Number of material Labels in AtlasMesh and AtlasMeshLabels do not match");
      }
    }

  auto elc_vals_in_table = makeHomogeneousVariableList([&AtlasMeshLabels_vector, &output](size_t i)
  {
    return makeAnonymousVariableList(AtlasMeshLabels_vector[i], //label name
      formatStatistic((*output)(i, 0)), //average
      formatStatistic((*output)(i, 1)), //stddev
      formatStatistic((*output)(i, 2)), //min
      formatStatistic((*output)(i, 3)), //max
      formatCount((*output)(i, 4))); //element count
  }, AtlasMeshLabels_vector.size());

  auto statistics_table(boost::make_shared<Variable>(Name("Table"), elc_vals_in_table));

  return boost::make_tuple(output, statistics_table);
}

/// this function takes the (x,y,z) location of the user specified ROI and results in a std:vector<bool> that contains trues for ROI mesh elements and false for non-ROI mesh elements
std::vector<bool> GenerateROIStatisticsAlgorithm::statistics_based_on_xyz_coodinates(const FieldHandle mesh, const FieldHandle CoordinateSpace, double x, double y, double z, double radius, int target_material) const
{
  VField* vfield_coordspace = CoordinateSpace->vfield();
  VMesh* vmesh_coordspace = vfield_coordspace->vmesh();
  VField* vfield_atlas = mesh->vfield();
  VMesh* vmesh_atlas = vfield_atlas->vmesh();
  std::vector<bool> element_selection;
  long closest_atlas_element=-1;
  double distance=-1, mindis=std::numeric_limits<double>::max();
  Vector val;

  for (VMesh::Elem::index_type i=0; i < vmesh_coordspace->num_elems(); i++) /// loop over all tetrahedral elements (mesh)
  {
    vfield_coordspace->get_value(val,i);
    distance = std::sqrt((x-val[0])*(x-val[0])+(y-val[1])*(y-val[1])+(z-val[2])*(z-val[2]));
    if (distance < mindis)
    {
      closest_atlas_element=(long)i;
      mindis=distance;
    }
  }

  if (mindis>1)
  {
    std::ostringstream ostr1;
    ostr1 << "Distance from provided point (lower table in GUI: x,y,z) is more then 1 (=" << mindis << ") distance units away from provided coordinate space (coordinates defined as element data)." <<  std::endl;
    THROW_ALGORITHM_INPUT_ERROR(ostr1.str());

  }

  Point p;
  VMesh::Elem::index_type atlas_index = closest_atlas_element;
  vmesh_coordspace->get_center(p,atlas_index);

  double x1=p.x(), y1=p.y(), z1=p.z();

  long count_loop=0;

  for (VMesh::Elem::index_type i=0; i < vmesh_atlas->num_elems(); i++) /// loop over all tetrahedral elements (mesh)
  {
    if (target_material!=0)  /// was the target material (Atlas Material #) provided not the default, so "-1"
    {
      int current_material=-1;
      vfield_atlas->get_value(current_material, i);
      if (target_material==current_material) /// if the current material is in the defined spherical ROI and of the material we are looking for?
      {
        VMesh::Elem::index_type tmp = count_loop;
        vmesh_atlas->get_center(p,tmp);

        distance = sqrt((x1-p.x())*(x1-p.x())+(y1-p.y())*(y1-p.y())+(z1-p.z())*(z1-p.z()));
        if (distance > radius)
        {
          element_selection.push_back(false);
        } else
        {
          element_selection.push_back(true);
        }
      } else
      {
        element_selection.push_back(false);
      }
    } else
    {     /// in the else close look for materials in the ROI
      VMesh::Elem::index_type tmp = count_loop;
      vmesh_atlas->get_center(p,tmp);
      distance = sqrt((x1-p.x())*(x1-p.x())+(y1-p.y())*(y1-p.y())+(z1-p.z())*(z1-p.z()));
      if ( distance > radius)
      {
        element_selection.push_back(false);
      } else
      {
        element_selection.push_back(true);
      }
    }
    count_loop++;
  }

  return element_selection;
}

/// this function cuts the atlas label string into pieces (std::vector) based on the semicolon
std::vector<std::string> GenerateROIStatisticsAlgorithm::ConvertInputAtlasStringIntoVector(const std::string& atlasLabels) const
{
  std::vector<std::string> result;
  auto atlasLabelsTrimmed = atlasLabels;
  boost::trim_if(atlasLabelsTrimmed, boost::is_any_of(";")); /// use boost's trim function to get rid of all additional semicolons or
  boost::split(result,atlasLabelsTrimmed,boost::is_any_of(";")); /// use boost's trim function to cut the string

  return result;
}

AlgorithmOutput GenerateROIStatisticsAlgorithm::run(const AlgorithmInput& input) const
{

  auto mesh_ = input.get<Field>(MeshDataOnElements);
  auto physicalUnit_ = input.get<Datatypes::String>(PhysicalUnit);
  auto atlasMesh_ = input.get<Field>(AtlasMesh);
  auto atlasMeshLabels_ = input.get<Datatypes::String>(AtlasMeshLabels);
  auto coordinateSpace_ = input.get<Field>(CoordinateSpace);
  auto coordinateLabel_ = input.get<Datatypes::String>(CoordinateSpaceLabel);
  auto roiSpec = input.get<DenseMatrix>(SpecifyROI);

  /// In the following check the validity of the inputs
  if (!mesh_)
    THROW_ALGORITHM_INPUT_ERROR("First input (mesh) is empty.");

  if (!atlasMesh_)
    THROW_ALGORITHM_INPUT_ERROR("Third input (atlas mesh) is empty.");

  FieldInformation fi(mesh_);

  if (!fi.is_constantdata())
    THROW_ALGORITHM_INPUT_ERROR("First input (mesh) requires the data to be on the elements.");

  // making sure the field contains data
  VField* vfield1 = mesh_->vfield();
  if (vfield1->is_nodata())
    THROW_ALGORITHM_INPUT_ERROR("First input field (mesh) contained no data.");

  // making sure the field is not in vector format
  if (!vfield1->is_scalar())
    THROW_ALGORITHM_INPUT_ERROR("First input field needs to have scalar data.");

  FieldInformation fi2(atlasMesh_);

  if (!fi2.is_constantdata())
    THROW_ALGORITHM_INPUT_ERROR("First input (mesh) requires the data to be on the elements.");

  // making sure the field contains data
  VField* vfield2 = atlasMesh_->vfield();
  if (vfield2->is_nodata())
    THROW_ALGORITHM_INPUT_ERROR("First input field (mesh) contained no data.");

  // making sure the field is not in vector format
  if (!vfield2->is_scalar())
    THROW_ALGORITHM_INPUT_ERROR("First input field needs to have scalar data.");

  if(vfield1->vmesh()->num_elems()<1 && vfield2->vmesh()->num_elems()<1)
    THROW_ALGORITHM_INPUT_ERROR("First (mesh) or second (AtlasMesh) input field does not contain elements.");

  if(vfield2->vmesh()->num_elems() !=  vfield1->vmesh()->num_elems())
    THROW_ALGORITHM_INPUT_ERROR(" Number of mesh elements of first input and third input does not match.");

  DenseMatrixHandle statistics;
  VariableHandle Statisticstable;

  /// since there are so many optional inputs to the module decide based on them and call the run function
  const std::string& atlasMeshLabelsStr = atlasMeshLabels_ == nullptr ? std::string("") : atlasMeshLabels_->value();
  const FieldHandle coorspace_ = coordinateSpace_ == nullptr ? FieldHandle() : coordinateSpace_;
  const DenseMatrixHandle roiSpec_ = roiSpec == nullptr ? DenseMatrixHandle() : roiSpec;
  boost::tie(statistics, Statisticstable) = run(mesh_, atlasMesh_, coorspace_, atlasMeshLabelsStr, roiSpec_);

  /// no statistics output? something went wrong then
  if (!statistics)
  {
    THROW_ALGORITHM_INPUT_ERROR(" Statistics output is null pointer! ");
  }

  AlgorithmOutput output;
  output[StatisticalResults] = statistics;
  output.setAdditionalAlgoOutput(Statisticstable);

  return output;
}
