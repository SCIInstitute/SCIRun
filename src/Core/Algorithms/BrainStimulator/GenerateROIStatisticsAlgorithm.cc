/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
#include <Core/Logging/Log.h>
#include <string>

//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
#include <iostream>
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;
using namespace SCIRun::Core::Logging;

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
  addParameter(PhysicalUnitStr, "");
  addParameter(CoordinateSpaceLabelStr, "");
}

AlgorithmParameterName GenerateROIStatisticsAlgorithm::StatisticsRowName(int i) { return AlgorithmParameterName(Name("elc"+boost::lexical_cast<std::string>(i)));}

boost::tuple<DenseMatrixHandle, Variable> GenerateROIStatisticsAlgorithm::run(FieldHandle mesh, FieldHandle AtlasMesh, const FieldHandle CoordinateSpace, const std::string& AtlasMeshLabels, const Datatypes::DenseMatrixHandle specROI) const
{
 DenseMatrixHandle output;

 VField* vfield1 = mesh->vfield();
 VField* vfield2 = AtlasMesh->vfield();
 
 std::set<int> labelSet;
 
 for (VMesh::Elem::index_type i=0; i < vfield2->vmesh()->num_elems(); i++) // loop over all tetrahedral elements (mesh)
 {
   int Label;
   vfield2->get_value(Label, i);
   labelSet.insert(Label);
 }
  
 std::set<bool> element_selection;
 
 // CoordinateSpace is provided and coodinates? if not dont go in here
 if (CoordinateSpace != nullptr && specROI != nullptr) //input provided in SpecifyROI_tabWidget-GUI? (x,y,z,radius)
 {
  double x=0,y=0,z=0,radius=-1,material=-1;
  if ( (*specROI).ncols()==1 && (*specROI).nrows()==5 )
  {  
   x=(*specROI)(0,0);
   y=(*specROI)(1,0);
   z=(*specROI)(2,0);
   material=(*specROI)(3,0);
   radius=(*specROI)(4,0);
   
   if (radius>0)
   {
     element_selection = statistics_based_on_xyz_coodinates(mesh, CoordinateSpace, labelSet, x, y, z, radius, material);
   } else
   {
     THROW_ALGORITHM_INPUT_ERROR("Radius needs to be > 0 ");  
   }
  }
 }

 const size_t number_of_atlas_materials = labelSet.size();
 std::vector<int> labelVector(labelSet.begin(), labelSet.end());

 std::ostringstream ostr;
 std::copy(labelSet.begin(), labelSet.end(), std::ostream_iterator<int>(ostr, ", "));
 LOG_DEBUG("Sorted set of label numbers: " << ostr.str() << std::endl);
  
 std::vector<double> value_avr(number_of_atlas_materials);
 std::vector<int> value_count(number_of_atlas_materials);
 std::vector<double> value_min(number_of_atlas_materials);
 std::vector<double> value_max(number_of_atlas_materials);
 std::vector<double> value_std(number_of_atlas_materials); 
 std::vector<double> Sxsqr(number_of_atlas_materials);
 std::vector<double> stddev(number_of_atlas_materials);
 std::vector<double> var(number_of_atlas_materials);
 
 for (VMesh::Elem::index_type i=0; i < vfield1->vmesh()->num_elems(); i++) // loop over all tetrahedral elements (AtlasMesh)
 {
   double value = 0;
   vfield1->get_value(value, i); 
   
   int Label = 0;
   vfield2->get_value(Label, i);
   
   for (VMesh::Elem::index_type j=0; j < number_of_atlas_materials; j++)
   {
     if (Label==labelVector[j]) 
     {
       value_avr[j] += value; 
       value_count[j]++;   
       if (value>value_max[j]) value_max[j]=value;
       if (value<value_min[j]) value_min[j]=value;
       Sxsqr[j]+=value*value;
     }
   }
 }

 output = DenseMatrixHandle(new DenseMatrix(number_of_atlas_materials, 4));
 
 //efficient way to compute std dev. in just one loop over all mesh elements: sqrt ( 1/(n-1) (Sx^2 - avr Sx + n avr^2 )
 for (VMesh::Elem::index_type j=0; j < number_of_atlas_materials; j++)
 {
   double Sx=value_avr[j];
   value_avr[j]/=value_count[j]; 
   var[j]=static_cast<double>(1./(value_count[j]-1)*(Sxsqr[j]-2*value_avr[j]*Sx+value_count[j]*value_avr[j]*value_avr[j]));
   stddev[j]=static_cast<double>(std::sqrt(var[j]));
   (*output)(j,0)=value_avr[j];
   (*output)(j,1)=stddev[j];
   (*output)(j,2)=value_min[j];
   (*output)(j,3)=value_max[j];   
 }  

  std::vector<std::string> AtlasMeshLabels_vector;
  if (!AtlasMeshLabels.empty())
  {
   AtlasMeshLabels_vector = ConvertInputAtlasStringIntoVector(AtlasMeshLabels); 
  }
   
 if (AtlasMeshLabels_vector.size()==0 && number_of_atlas_materials>0)
 {
   AtlasMeshLabels_vector.resize(number_of_atlas_materials);
   for (int i=0; i<number_of_atlas_materials; i++)
   {
    AtlasMeshLabels_vector[i]=boost::lexical_cast<std::string>(i+1);
   }
 }
 else
  if (AtlasMeshLabels_vector.size() != number_of_atlas_materials)
   {
    THROW_ALGORITHM_INPUT_ERROR("Number of material Labels in AtlasMesh and AtlasMeshLabels do not match"); 
   } 
     
  std::vector<AlgorithmParameter> elc_vals_in_table;
  for (int i=0; i<AtlasMeshLabels_vector.size(); i++)
  {
    std::vector<AlgorithmParameter> tmp;
    tmp.push_back(AlgorithmParameter(Name("name"), AtlasMeshLabels_vector[i])); //label name
    
    tmp.push_back(AlgorithmParameter(Name("col0"), boost::str(boost::format("%.3f") % (*output)(i,0)))); //average
    tmp.push_back(AlgorithmParameter(Name("col1"), boost::str(boost::format("%.3f") % (*output)(i,1)))); //stddev
    tmp.push_back(AlgorithmParameter(Name("col2"), boost::str(boost::format("%.3f") % (*output)(i,2)))); //min
    tmp.push_back(AlgorithmParameter(Name("col3"), boost::str(boost::format("%.3f") % (*output)(i,3)))); //max
    AlgorithmParameter row_i(Name("row" + boost::lexical_cast<std::string>(i)), tmp);
    elc_vals_in_table.push_back(row_i);
  }
  
  AlgorithmParameter statistics_table(Name("Table"), elc_vals_in_table); 
 
 return boost::make_tuple(output, statistics_table);
}

std::set<bool> GenerateROIStatisticsAlgorithm::statistics_based_on_xyz_coodinates(const FieldHandle mesh, const FieldHandle CoordinateSpace, std::set<int>& labelSet, double x, double y, double z, double radius, int material) const
{
  std::set<bool> element_selection;
  VField* vfield1 = mesh->vfield();
  VMesh* vmesh = vfield1->vmesh();
  
  long count_the_spec_materials=0;
  
  if (material != 0)
  {
   for (std::set<int>::iterator it=labelSet.begin(); it!=labelSet.end(); ++it)
    {
     if (*it==material)
      {
       count_the_spec_materials++;
      }
    }
  }
    
  if (count_the_spec_materials==0)
  {
    THROW_ALGORITHM_INPUT_ERROR("Specified material could not be found");
  }
  
  DenseMatrixHandle element_centers(material != 0 ? new DenseMatrix(count_the_spec_materials,3) : new DenseMatrix(vmesh->num_elems(),3));  
  
  if (material == 0)
  { 
   Point p;
   for (VMesh::Elem::index_type i=0; i < vmesh->num_elems(); i++) // loop over all tetrahedral elements of the mesh to be analyzed
   {
    vmesh->get_center(p,i);
    (*element_centers)(i,0) = p.x();
    (*element_centers)(i,1) = p.y();
    (*element_centers)(i,2) = p.z();
   }
  } else
  {
   Point p;
   long count_loop=0, count_found_labels=0;
   for (std::set<int>::iterator it=labelSet.begin(); it!=labelSet.end(); ++it)
   {
    if (*it==material)
    {
      VMesh::Elem::index_type tmp = count_loop;
      vmesh->get_center(p,tmp);
      (*element_centers)(count_found_labels,0) = p.x();
      (*element_centers)(count_found_labels,1) = p.y();
      (*element_centers)(count_found_labels,2) = p.z();
      count_found_labels++;
    }
    count_loop++;
   }
  }
  //if material is provided reduce label vector that only this material remains

  return element_selection;
}


std::vector<std::string> GenerateROIStatisticsAlgorithm::ConvertInputAtlasStringIntoVector(const std::string& atlasLabels) const
{
  std::vector<std::string> result;
  auto atlasLabelsTrimmed = atlasLabels;
  boost::trim_if(atlasLabelsTrimmed, boost::is_any_of(";"));
  boost::split(result,atlasLabelsTrimmed,boost::is_any_of(";"));
  
  return result;
}

AlgorithmOutput GenerateROIStatisticsAlgorithm::run_generic(const AlgorithmInput& input) const
{

  auto mesh_ = input.get<Field>(MeshDataOnElements);
  auto physicalUnit_ = input.get<Datatypes::String>(PhysicalUnit);
  auto atlasMesh_ = input.get<Field>(AtlasMesh);
  auto atlasMeshLabels_ = input.get<Datatypes::String>(AtlasMeshLabels);
  auto coordinateSpace_ = input.get<Field>(CoordinateSpace);
  auto coordinateLabel_ = input.get<Datatypes::String>(CoordinateSpaceLabel);  
  auto roiSpec = input.get<DenseMatrix>(SpecifyROI);
 
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
  Variable Statisticstable;
  
  const std::string& atlasMeshLabelsStr = atlasMeshLabels_ == nullptr ? std::string("") : atlasMeshLabels_->value();
  const FieldHandle coorspace_ = coordinateSpace_ == nullptr ? FieldHandle() : coordinateSpace_;
  const DenseMatrixHandle roiSpec_ = roiSpec == nullptr ? DenseMatrixHandle() : roiSpec;
  boost::tie(statistics, Statisticstable) = run(mesh_, atlasMesh_, coorspace_, atlasMeshLabelsStr, roiSpec_);
  
  if (statistics == nullptr)
  {
    THROW_ALGORITHM_INPUT_ERROR(" Statistics output is null pointer! "); 
  }

  AlgorithmOutput output;
  output[StatisticalResults] = statistics;
  output.setAdditionalAlgoOutput(Statisticstable);
  
  return output;
}
