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

   Author: Moritz Dannhauer, Spencer Frisby
   Date:   September 2014
*/
///@file SetupRHSforTDCSandTMS
///@brief 
/// This module sets up TDCS problem by providing the right hand side vector (controlled by GUI) and inputs for the modules: AddKnownsToLinearSystem, BuildTDCSMatrix. 
///
///@author
/// Moritz Dannhauer, Spencer Frisby
///
///@details
/// .The module gets 4 inputs the tetrahedral mesh (first), triangle surfaces (as part of the first input) for the scalp (second) and electrodes sponges (third) as well as the a matrix (output of ElectrodeCoilSetup)
///  that contains the center location and thickness of the electrode sponge. The first output contains the knowns (reference zeros node) of the linear system (third input of AddKnownsToLinearSystem). 
///  The zero potential node is set to be the first mesh node by default, which can be chosen arbitrarily in the GUI by the user. The second until fifth output prepares the inputs for the BuildTDCSMatrix module that
///  combines the FEM stiffness matrix with the boundary conditions of the complete electrode model. The electrical impedances can be provided in the GUI for each electrode individually.
///  The module needs to identify the surface of the sponge which makes up the major part of the algorithmic module implementation. 
///  The sixth output creates the right hand side (RHS) vector that can be provided by the GUI. The electrical charges (ingoing and outgoing current intensities) need to sum up zero. 
///  If you execute the module once the module will graphically only present the number of electrodes defined in the input data.
///  The complete electrode model solves the nodal and electrode potentials in an agumented linear system the potential solution (output of SolveLinearSystem) needs to adjuested.
///  Therefore, a seventh output is provided that contains the relevant indeces to be used for the module 'SelectSubMatrix'. 
///  Since the identification of the contact surface between electrode sponge and the electrode itself (the sponge surface top is assumed to be that) is a difficult geometrical process the eighth 
///  output provides a triangle mesh that can be used for visual inspection (->ShowField->ViewScene).

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/BrainStimulator/SetupRHSforTDCSandTMSAlgorithm.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <boost/lexical_cast.hpp>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/SplitByConnectedRegion.h>
#include <Core/Algorithms/Legacy/Fields/DomainFields/SplitFieldByDomainAlgo.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(BrainStimulator, ElectrodeTableValues);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ImpedanceTableValues);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ELECTRODE_VALUES);
ALGORITHM_PARAMETER_DEF(BrainStimulator, IMPEDANCE_VALUES);
ALGORITHM_PARAMETER_DEF(BrainStimulator, refnode);
ALGORITHM_PARAMETER_DEF(BrainStimulator, normal_dot_product_bound);
ALGORITHM_PARAMETER_DEF(BrainStimulator, pointdistancebound);
ALGORITHM_PARAMETER_DEF(BrainStimulator, number_of_electrodes);

AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::MESH("MESH");
AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::SCALP_TRI_SURF_MESH("SCALP_TRI_SURF_MESH");
AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_TRI_SURF_MESH("ELECTRODE_TRI_SURF_MESH");
AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_SPONGE_LOCATION_AVR("ELECTRODE_SPONGE_LOCATION_AVR");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_ELEMENT("ELECTRODE_ELEMENT");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_ELEMENT_TYPE("ELECTRODE_ELEMENT_TYPE");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_ELEMENT_DEFINITION("ELECTRODE_ELEMENT_DEFINITION");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_CONTACT_IMPEDANCE("ELECTRODE_CONTACT_IMPEDANCE");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::RHS("RHS");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::LHS_KNOWNS("LHS_KNOWNS");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_SPONGE_SURF("ELECTRODE_SPONGE_SURF");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::SELECTMATRIXINDECES("SELECTMATRIXINDECES");

AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::ElecrodeParameterName(int i) { return AlgorithmParameterName(Name("elc"+boost::lexical_cast<std::string>(i)));}
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::ElecrodeImpedanceParameterName(int i) { return AlgorithmParameterName(Name("imp_elc"+boost::lexical_cast<std::string>(i)));}

SetupRHSforTDCSandTMSAlgorithm::SetupRHSforTDCSandTMSAlgorithm()
{
  addParameter(Parameters::ELECTRODE_VALUES, 0); // just a default value, will be replaced with vector
  addParameter(Parameters::IMPEDANCE_VALUES, 0);
  addParameter(Parameters::number_of_electrodes, 128);
  addParameter(Parameters::refnode, 0);  
  addParameter(Parameters::normal_dot_product_bound, 0.7);
  addParameter(Parameters::pointdistancebound, 0.0001);
}

AlgorithmOutput SetupRHSforTDCSandTMSAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto mesh = input.get<Field>(MESH);

  auto all_elc_values = get(Parameters::ELECTRODE_VALUES).getList();
  auto all_imp_elc_values = get(Parameters::IMPEDANCE_VALUES).getList();
  
  if (all_elc_values.size()!=all_imp_elc_values.size())
  {
    THROW_ALGORITHM_PROCESSING_ERROR("Internal Error: electrode information could not retrieved from GUI.");
  }
  
  /// consistency checks to validate the GUI<->Algorithm data transfer
  for (int i=0; i<all_elc_values.size(); i++)
  {
    auto elecName = all_elc_values[i].nameForXml(); 
   // auto elecValue = all_elc_values[i].getDouble(); // why is not that used?
    auto expectedElecName = SetupRHSforTDCSandTMSAlgorithm::ElecrodeParameterName(i); // ElecrodeParameterName(i);
    if(elecName.name_.compare(expectedElecName.name_) != 0) // if so, electrodes are being stored out of order.
      THROW_ALGORITHM_PROCESSING_ERROR("Values are being stored out of order!");
      
    auto impelecName = all_imp_elc_values[i].nameForXml(); 
    auto expectedImpElecName = SetupRHSforTDCSandTMSAlgorithm::ElecrodeImpedanceParameterName(i);
    if(elecName.name_.compare(expectedElecName.name_) != 0) // if so, electrodes are being stored out of order.
      THROW_ALGORITHM_PROCESSING_ERROR("Impedance values are being stored out of order!");
  }
  
  auto scalp_tri_surf = input.get<Field>(SCALP_TRI_SURF_MESH);
  auto elc_tri_surf = input.get<Field>(ELECTRODE_TRI_SURF_MESH);
  
  if(!elc_tri_surf)
  {
    THROW_ALGORITHM_PROCESSING_ERROR("Input Error: electrode triangle surface is not allocated.");   
  }
  
  if(!scalp_tri_surf)
  {
    THROW_ALGORITHM_PROCESSING_ERROR("Input Error: Scalp triangle surface is not allocated.");   
  }  
  
  // obtaining number of electrodes
  DenseMatrixHandle elc_sponge_location = matrix_convert::to_dense(input.get<Matrix>(ELECTRODE_SPONGE_LOCATION_AVR));
  if (!elc_sponge_location)
  {
   THROW_ALGORITHM_PROCESSING_ERROR("Electrode sponges matrix (center locations) is not allocated."); 
  }
  if (elc_sponge_location->ncols()!=4 || elc_sponge_location->nrows()<2)
  {
   THROW_ALGORITHM_PROCESSING_ERROR("Electrode sponges matrix needs to have dimension #sponges x 4 (#sponges>=2)"); 
  }

  int num_of_elc = elc_sponge_location->nrows();
  
  AlgorithmOutput output; 
  
  DenseMatrixHandle lhs_knowns, elc_element, elc_element_typ, elc_element_def, elc_contact_imp, rhs, selectmatrixind;
  FieldHandle elec_sponge_surf;
  boost::tie(lhs_knowns, elc_element, elc_element_typ, elc_element_def, elc_contact_imp, rhs, elec_sponge_surf, selectmatrixind) = run(mesh, all_elc_values, all_imp_elc_values, num_of_elc, scalp_tri_surf, elc_tri_surf, elc_sponge_location);
  
  output[LHS_KNOWNS] = lhs_knowns;
  output[ELECTRODE_ELEMENT] = elc_element;
  output[ELECTRODE_ELEMENT_TYPE] = elc_element_typ;
  output[ELECTRODE_ELEMENT_DEFINITION] = elc_element_def;
  output[ELECTRODE_CONTACT_IMPEDANCE] = elc_contact_imp;
  output[RHS] = rhs;
  output[SELECTMATRIXINDECES] = selectmatrixind;
  output[ELECTRODE_SPONGE_SURF] = elec_sponge_surf;
  
  return output;
}

/// replace this code with calls to splitfieldbyconnectedregion, clipfieldby* if available for SCIRun5 
boost::tuple<DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle, FieldHandle> SetupRHSforTDCSandTMSAlgorithm::create_lhs(FieldHandle mesh, const std::vector<Variable>& impelc, FieldHandle scalp_tri_surf, FieldHandle elc_tri_surf, DenseMatrixHandle elc_sponge_location) const
{
 VMesh*  mesh_vmesh = mesh->vmesh();
 VMesh::size_type mesh_num_nodes = mesh_vmesh->num_nodes();
 DenseMatrixHandle lhs_knows, elc_elem, elc_elem_typ, elc_elem_def, elc_con_imp;

 index_type refnode_number = get(Parameters::refnode).toInt();
 double normal_dot_product_bound_ = get(Parameters::normal_dot_product_bound).toDouble();
 double identical_node_location_differce = get(Parameters::pointdistancebound).toDouble();

 if (identical_node_location_differce<=0 || refnode_number<0 || normal_dot_product_bound_<0)   /// check for valid input data
 {
  THROW_ALGORITHM_PROCESSING_ERROR(" Gui values (refnode, point dis., surf shrink) should be > 0. ");
 }
  
 SplitFieldByConnectedRegionAlgo algo;
 algo.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), false);
 algo.set(SplitFieldByConnectedRegionAlgo::SortAscending(), false);
 std::vector<FieldHandle> result = algo.run(elc_tri_surf);
 
 if (result.size()<=0)
 {
    THROW_ALGORITHM_PROCESSING_ERROR(" Splitting input mesh into connected regions failed. ");
 }
 
 if (elc_sponge_location->ncols()!=4)
 {
   THROW_ALGORITHM_PROCESSING_ERROR(" ELECTRODE_SPONGE_LOCATION_AVR (4th module input) needs to have 4 columns (x,y,z,elc. sponge height).");
 }
 
 if( result.size() != elc_sponge_location->nrows())
 {
   THROW_ALGORITHM_PROCESSING_ERROR(" The number of electrode sponges (4th module input) does not match number of a splitted electrode surfaces (3rd module input).");
 }
 
 if ( refnode_number > mesh_num_nodes || refnode_number<0)
 {
    THROW_ALGORITHM_PROCESSING_ERROR(" Reference node exceeds number of FEM nodes. ");
 }
 
 /// prepare LHS_KNOWNS which is the an input to addknownstolinearsystem (called x) besides the stiffness matrix
 lhs_knows=boost::make_shared<DenseMatrix>(mesh_num_nodes,1);
 for(VMesh::Node::index_type idx=0; idx<mesh_num_nodes; idx++)
 {
   (*lhs_knows)(idx,0)=std::numeric_limits<double>::quiet_NaN();
 }
 (*lhs_knows)(refnode_number,0)=0;
 
 FieldInformation fieldinfo("TriSurfMesh", CONSTANTDATA_E, "int");
 FieldHandle elc_sponge_surf = CreateField(fieldinfo);      
 VMesh* elc_sponge_surf_vmesh = elc_sponge_surf->vmesh();  
 VField* elc_sponge_surf_vfld  = elc_sponge_surf->vfield();
 std::vector<double> field_values, impedances;
 
 /// map the electrode sponge center (CreateElectrodeCoil) to generated tDCS electrode geometry (Cleaver), 
 /// this mapping is meant to map the GUI inputs to actual electrode geometry by having a lookup table
 int electrode_sponges=elc_sponge_location->nrows();
 DenseMatrixHandle lookup(new DenseMatrix(electrode_sponges, 1));
 DenseMatrixHandle distances(new DenseMatrix(electrode_sponges, 1));
 VMesh::Node::index_type didx;
 double distance=0; 
 for (long i=0;i<electrode_sponges;i++)
 {
  Point elc((*elc_sponge_location)(i,0),(*elc_sponge_location)(i,1),(*elc_sponge_location)(i,2)),r;
  double min_dis=std::numeric_limits<double>::infinity();
  long found_index=std::numeric_limits<double>::quiet_NaN();
  for(long j=0;j<result.size();j++)
  {
   VMesh*  tmp_mesh = result[j]->vmesh();
   tmp_mesh->synchronize(Mesh::NODE_LOCATE_E); 
   
   tmp_mesh->find_closest_node(distance,r,didx,elc);
   if (distance<min_dis)
   {
     min_dis=distance;
     found_index=j;
   }   
  }
  (*lookup)(i,0)=found_index;
  (*distances)(i,0)=min_dis;
 }
 
 /// throw error if the geometry is too far away from predicted location - further away than sponge thickness
 for (long i=0;i<elc_sponge_location->nrows();i++)
 {
   if ((*distances)(i,0)>(*elc_sponge_location)(i,3))
   {
    std::ostringstream ostr1;
    ostr1 << " distance to electrode " << i << " sponge  = " <<  (*distances)(i,0) << " exceeds defined limit of " << (*elc_sponge_location)(i,3) << std::endl;
    THROW_ALGORITHM_PROCESSING_ERROR(ostr1.str());
   }
 } 

 /// determine intersection of electrode sponge and scalp; as well as center of electrode sponge/scalp surface
 FieldInformation fi("PointCloudMesh",0,"double");
 FieldHandle sponge_geometry_centers=CreateField(fi);
 VMesh* mesh_sponge_geometry_centers = sponge_geometry_centers->vmesh();
 
 VMesh* mesh_scalp_tri_surf  = scalp_tri_surf->vmesh(); 
 Point p,q; Vector norm; 
 VMesh::Node::index_type node_ind;
 distance=std::numeric_limits<double>::infinity(); 
 DenseMatrixHandle sponge_center_pojected_onto_scalp_index(new DenseMatrix(result.size(), 1));
 DenseMatrixHandle sponge_center_pojected_onto_scalp_normal(new DenseMatrix(result.size(), 3));
 DenseMatrixHandle sponge_center_pojected_onto_scalp(new DenseMatrix(result.size(), 3));
 
 mesh_scalp_tri_surf->synchronize(Mesh::NODE_LOCATE_E);

 for(long i=0;i<result.size();i++)
 {
  VMesh*  tmp_mesh = result[i]->vmesh();
  tmp_mesh->synchronize(Mesh::NODE_LOCATE_E);
  double avr_x=0,avr_y=0,avr_z=0;
  long number_electrode_nodes=tmp_mesh->num_nodes();
  
  FieldInformation fi2("PointCloudMesh",0,"double");
  FieldHandle tmp_field=CreateField(fi2);
  VMesh* tmp_field_mesh = tmp_field->vmesh();
 
  for (VMesh::Node::index_type idx=0; idx<number_electrode_nodes; idx++)
  {
    tmp_mesh->get_center(p,idx); 
    avr_x+=p.x();
    avr_y+=p.y();
    avr_z+=p.z();
    if(mesh_scalp_tri_surf->find_closest_node(distance,q,node_ind,p))
    {
     if (distance<identical_node_location_differce)
     {  
      tmp_field_mesh->add_point(q);
     } 
    }
  }
  
  avr_x/=(number_electrode_nodes);
  avr_y/=(number_electrode_nodes);
  avr_z/=(number_electrode_nodes);
  
  Point avr_sponge_geometry(avr_x,avr_y,avr_z);
  mesh_sponge_geometry_centers->add_point(avr_sponge_geometry);
  tmp_field_mesh->synchronize(Mesh::NODE_LOCATE_E);
  tmp_field_mesh->find_closest_node(distance,q,node_ind,avr_sponge_geometry);
    
  (*sponge_center_pojected_onto_scalp)(i,0)=q.x();
  (*sponge_center_pojected_onto_scalp)(i,1)=q.y();
  (*sponge_center_pojected_onto_scalp)(i,2)=q.z();
  
  mesh_scalp_tri_surf->find_closest_node(distance,p,node_ind,q);
  (*sponge_center_pojected_onto_scalp_index)(i,0)=node_ind;
 }
 
 /// determine normal of scalp/electrode sponge inferface center 
 mesh_scalp_tri_surf->synchronize(Mesh::NORMALS_E);
 VMesh::Elem::array_type ca;
 for(long j=0;j<result.size();j++)
 { 
  VMesh::Node::index_type ii((*sponge_center_pojected_onto_scalp_index)(j,0));
  mesh_scalp_tri_surf->get_normal(norm,ii);
  double x=norm.x(),y=norm.y(),z=norm.z();
  
  double normal_mag=sqrt(x*x+y*y+z*z); /// normalize vector normal 
  x/=normal_mag;
  y/=normal_mag;
  z/=normal_mag;
  
  (*sponge_center_pojected_onto_scalp_normal)(j,0)=x;
  (*sponge_center_pojected_onto_scalp_normal)(j,1)=y;
  (*sponge_center_pojected_onto_scalp_normal)(j,2)=z;
 }
 
 VMesh* mesh_elc_tri_surf = scalp_tri_surf->vmesh();
 mesh_elc_tri_surf->synchronize(Mesh::NODE_LOCATE_E); 
  
 int nr_elc_sponge_triangles=0;

 FieldInformation fi3("PointCloudMesh",0,"double");
 FieldHandle estimated_sponge_top_center_points=CreateField(fi3);
 VMesh*  estimated_sponge_top_center_points_vmesh = estimated_sponge_top_center_points->vmesh();
 
 /// retrieve electrode thickness from last input by using the lookup table
 for(int k=0;k<sponge_center_pojected_onto_scalp_normal->nrows();k++)
 { 
  bool normal_traveling_direction_needs_to_be_positive=true;
  double elc_thickness=0;
  bool found=false;
  
  for (long j=0;j<lookup->nrows();j++)
  {
    if ((*lookup)(j,0)==k) 
      {
        elc_thickness=(*elc_sponge_location)((*lookup)(j,0),3);
        found=true; 
      }      
  }

  if (!found)
  {
   THROW_ALGORITHM_PROCESSING_ERROR("Internal error: Lookup of mesh defined electrode (first input) and definition (fourth input) do not match - check your inputs!");  
  } 
  
  /// determine the outgoing normal of the scalp to find sponge/electrode surface
  Point o1((*sponge_center_pojected_onto_scalp)(k,0)+(*sponge_center_pojected_onto_scalp_normal)(k,0)*elc_thickness/2,(*sponge_center_pojected_onto_scalp)(k,1)+(*sponge_center_pojected_onto_scalp_normal)(k,1)*elc_thickness/2,(*sponge_center_pojected_onto_scalp)(k,2)+(*sponge_center_pojected_onto_scalp_normal)(k,2)*elc_thickness/2);
  Point o2((*sponge_center_pojected_onto_scalp)(k,0)-(*sponge_center_pojected_onto_scalp_normal)(k,0)*elc_thickness/2,(*sponge_center_pojected_onto_scalp)(k,1)-(*sponge_center_pojected_onto_scalp_normal)(k,1)*elc_thickness/2,(*sponge_center_pojected_onto_scalp)(k,2)-(*sponge_center_pojected_onto_scalp_normal)(k,2)*elc_thickness/2); 
     
  mesh_sponge_geometry_centers->synchronize(Mesh::NODE_LOCATE_E);
  mesh_sponge_geometry_centers->get_center(p,(VMesh::Node::index_type)k);
 
  double d1=sqrt((p.x()-o1.x())*(p.x()-o1.x())+(p.y()-o1.y())*(p.y()-o1.y())+(p.z()-o1.z())*(p.z()-o1.z()));
  double d2=sqrt((p.x()-o2.x())*(p.x()-o2.x())+(p.y()-o2.y())*(p.y()-o2.y())+(p.z()-o2.z())*(p.z()-o2.z()));
   
  normal_traveling_direction_needs_to_be_positive=true;
  
  double x=0,y=0,z=0;
  
  if (d1==d2)
  {
    THROW_ALGORITHM_PROCESSING_ERROR("Internal error: Criteria to find sponge top and bottom failed!");
  } else
  if (d1>d2)
  { 
    normal_traveling_direction_needs_to_be_positive=false; 
    x=(*sponge_center_pojected_onto_scalp)(k,0)-(*sponge_center_pojected_onto_scalp_normal)(k,0)*elc_thickness;
    y=(*sponge_center_pojected_onto_scalp)(k,1)-(*sponge_center_pojected_onto_scalp_normal)(k,1)*elc_thickness;
    z=(*sponge_center_pojected_onto_scalp)(k,2)-(*sponge_center_pojected_onto_scalp_normal)(k,2)*elc_thickness;
  } else
  {
    normal_traveling_direction_needs_to_be_positive=true;
    x=(*sponge_center_pojected_onto_scalp)(k,0)+(*sponge_center_pojected_onto_scalp_normal)(k,0)*elc_thickness;
    y=(*sponge_center_pojected_onto_scalp)(k,1)+(*sponge_center_pojected_onto_scalp_normal)(k,1)*elc_thickness;
    z=(*sponge_center_pojected_onto_scalp)(k,2)+(*sponge_center_pojected_onto_scalp_normal)(k,2)*elc_thickness;
  }
  
  VMesh*  tmp_mesh = result[k]->vmesh(); 
  VField* tmp_fld = result[k]->vfield();
  tmp_mesh->synchronize(Mesh::NORMALS_E);  
  
  Point o3(x,y,z),o4; //electrode surface point = additional criterial to identify the sponge top/electrode surface  
  tmp_mesh->synchronize(Mesh::NODE_LOCATE_E);
  tmp_mesh->find_closest_node(distance,o4,node_ind,o3);
  estimated_sponge_top_center_points_vmesh->add_point(o4);
  
  Vector sponge_outwards_normal;
  if (normal_traveling_direction_needs_to_be_positive) /// force normals to point towards electrode
  {
   sponge_outwards_normal.x((*sponge_center_pojected_onto_scalp_normal)(k,0));
   sponge_outwards_normal.y((*sponge_center_pojected_onto_scalp_normal)(k,1));
   sponge_outwards_normal.z((*sponge_center_pojected_onto_scalp_normal)(k,2));
  } else
  {
   sponge_outwards_normal.x(-(*sponge_center_pojected_onto_scalp_normal)(k,0));
   sponge_outwards_normal.y(-(*sponge_center_pojected_onto_scalp_normal)(k,1));
   sponge_outwards_normal.z(-(*sponge_center_pojected_onto_scalp_normal)(k,2));  
  }  
  
  /// determine which sponge electrode triangles make up sponge top (using the scalp outgoing normal)
  /// the critia to control this can be adjusted in GUI "elc. normal criteria", -1..1, smaller values more electrode surface
  VMesh::coords_type center;
  VMesh::Face::size_type nrows;
  tmp_mesh->size(nrows);
  for(VMesh::Face::index_type idx=0; idx<nrows; idx++)
  {
   tmp_mesh->get_element_center(center);
   tmp_mesh->get_normal(norm,center,VMesh::Elem::index_type(idx));
   double dot_product=Dot(norm, sponge_outwards_normal);
      
   if (dot_product>=normal_dot_product_bound_)
   {
    tmp_fld->set_value(static_cast<int>(special_label), idx);  
   } else
   {
    tmp_fld->set_value(static_cast<int>(0), idx); 
   }
  }
  
  /// use SplitFieldByDomainAlgo to find only labeled (based on elc. normal criteria) surfaces
  VMesh::Elem::index_type c_ind=0;
  SplitFieldByDomainAlgo algo2;
  FieldList output;  
  algo2.set(SplitFieldByDomainAlgo::SortBySize, true);
  algo2.set(SplitFieldByDomainAlgo::SortAscending, false);
  algo2.runImpl(result[k], output);
  
  bool found_sponge_surface=false;
  double diff=std::numeric_limits<double>::max();
  int electrode_surface=-1;
  
  /// find out which of the selected triangles (from previous step) are closest to electrode surface center
  for(long o=0;o<output.size();o++)
  {
   VField* tmp_splitbydomain_tri_field = output[o]->vfield();
   VMesh* tmp_splitbydomain_tri_mesh = output[o]->vmesh();
   int tmp_val=0;
   tmp_splitbydomain_tri_field->get_value(tmp_val,c_ind);
      
   estimated_sponge_top_center_points_vmesh->get_center(o3,(VMesh::Node::index_type)k); //electrode surface point = additional criterial to identify the sponge top/electrode surface 
   Point o3(x,y,z),o4;
   tmp_splitbydomain_tri_mesh->synchronize(Mesh::NODE_LOCATE_E);
   tmp_splitbydomain_tri_mesh->find_closest_node(distance,o4,node_ind,o3);
   
   if (o==0 && tmp_val != special_label)
   {
    remark(" The identified electrode surface seem not to be the biggest that has correct normals. This is unusual. Visualize the 8th (Field) output to make sure otherwise the simulation could be wrong !");    
   }
   
   if ( tmp_val == special_label)
   {    
    found_sponge_surface=true; 
    if (diff>distance)
    {
     electrode_surface=o;
     diff=electrode_surface;
    }
   }
  }
   
  /// use SplitFieldByConnectedRegionAlgo to make sure we have the right triangle surface 
  if (!found_sponge_surface)
  {
    THROW_ALGORITHM_PROCESSING_ERROR(" At least for one electrode the electrode/sponge interface could not identified.");
  } else
  {
    std::ostringstream ostr1;
    ostr1 << "Electrode surface center estimation and found location differ: " << distance << ". The bigger the value the more concerned you should be. A distance close to 0..1 is optimial. Visualize the 8th (Field) output to make sure otherwise the simulation could be wrong!"  <<  std::endl;    
    remark(ostr1.str()); 
    if (diff>=elc_thickness)
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Internal error: The electrode sponge surface center is too far away. ");   
    } 
    
    SplitFieldByConnectedRegionAlgo algo3;
    algo3.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), true);
    algo3.set(SplitFieldByConnectedRegionAlgo::SortAscending(), false);
    std::vector<FieldHandle> result3 = algo3.run(output[electrode_surface]);
    if (result3.size()>1)
    {
     remark(" This module found two electrode sponge surfaces that are not connected and took the one that has more elements. Visualize the 8th (Field) output to make sure otherwise the simulation could be wrong!");    
    }
    if (result3.size()==0)
    {
     THROW_ALGORITHM_PROCESSING_ERROR("Internal error: Cannot find any electrode sponge surface. ");
    }
    
    /// create eightht output
    FieldHandle tri_mesh = result3[0];
    VMesh* tri_mesh_vmesh = tri_mesh->vmesh();
    VMesh::Node::array_type onodes(3);
    for (VMesh::Node::index_type l=0; l<tri_mesh_vmesh->num_nodes(); l++)
    {
      Point point;
      tri_mesh_vmesh->get_center(p,l);
      elc_sponge_surf_vmesh->add_point(p); 
    }
    
    for (VMesh::Elem::index_type l=0; l<tri_mesh_vmesh->num_elems(); l++) 
    {
     VMesh::Node::array_type onodes(3); 
     tri_mesh_vmesh->get_nodes(onodes, l);
     onodes[0]+=nr_elc_sponge_triangles;
     onodes[1]+=nr_elc_sponge_triangles;
     onodes[2]+=nr_elc_sponge_triangles;
     elc_sponge_surf_vmesh->add_elem(onodes);
     field_values.push_back(k);
     impedances.push_back(impelc[k].toDouble());
    }  
    nr_elc_sponge_triangles+=tri_mesh_vmesh->num_nodes(); 
  }
 
 }
 
 /// create matrix outputs
 elc_sponge_surf_vfld->resize_values();
 elc_sponge_surf_vfld->set_values(field_values);
 
 elc_elem = boost::make_shared<DenseMatrix>(DenseMatrix::Zero(elc_sponge_surf_vmesh->num_elems(),1));
 elc_elem_def = boost::make_shared<DenseMatrix>(DenseMatrix::Zero(elc_sponge_surf_vmesh->num_elems(),4));
 elc_elem_typ = boost::make_shared<DenseMatrix>(DenseMatrix::Zero(elc_sponge_surf_vmesh->num_elems(),1));
 elc_con_imp = boost::make_shared<DenseMatrix>(DenseMatrix::Zero(elc_sponge_surf_vmesh->num_elems(),1));
 
 if (impedances.size() != elc_sponge_surf_vmesh->num_elems())
 {
    THROW_ALGORITHM_PROCESSING_ERROR("Internal error: Number of impedances and electrode contact surface elements do not match up. ");
 }
 
 if (elc_sponge_surf_vmesh->num_elems()!=field_values.size())
 {
   THROW_ALGORITHM_PROCESSING_ERROR("Internal error: internal field definition is "); 
 }
 
 mesh_vmesh->synchronize(Mesh::NODE_LOCATE_E);
 
 for(VMesh::Elem::index_type l=0; l<elc_sponge_surf_vmesh->num_elems(); l++)
 {
  (*elc_elem_typ)(l,0)=2; //define triangles to incject currents in TDCS simulations
  VMesh::Node::array_type onodes(3); 
  elc_sponge_surf_vmesh->get_nodes(onodes, l);
  
  elc_sponge_surf_vmesh->get_center(p,onodes[0]);
  mesh_vmesh->find_closest_node(distance,q,node_ind,p);
  (*elc_elem_def)(l,0)=node_ind;
  
  elc_sponge_surf_vmesh->get_center(p,onodes[1]);
  mesh_vmesh->find_closest_node(distance,q,node_ind,p); 
  (*elc_elem_def)(l,1)=node_ind;
  
  elc_sponge_surf_vmesh->get_center(p,onodes[2]);
  mesh_vmesh->find_closest_node(distance,q,node_ind,p);
  (*elc_elem_def)(l,2)=node_ind;
  
  (*elc_elem_def)(l,3)=0; 
  (*elc_elem)(l,0)=field_values[l]; 
  (*elc_con_imp)(l,0)=impedances[l];
 }
  
 return boost::make_tuple(lhs_knows, elc_elem, elc_elem_typ, elc_elem_def, elc_con_imp,elc_sponge_surf);
}

DenseMatrixHandle SetupRHSforTDCSandTMSAlgorithm::create_rhs(FieldHandle mesh, const std::vector<Variable>& elcs, int num_of_elc) const
{
  /// get the right-hand-side from GUI
  std::vector<Variable, std::allocator<Variable>> elcs_wanted; 
  for (int i=0; i<num_of_elc; i++)
    elcs_wanted.push_back(elcs[i]);

  VField* vfield = mesh->vfield();
 
  // making sure current magnitudes of the electrodes summed are greater than 10e-6
  double min_current = 0;
  for (int i=0; i<elcs_wanted.size(); i++)
  {
    double temp = elcs_wanted[i].toDouble();
    min_current += temp;
  }
  
  if (std::fabs(min_current) > electode_current_summation_bound) 
        remark("Summed electrode current intensities are greater than 1e-6 mA. The sum should be close to zero. ");  
  
  int node_elements  = vfield->vmesh()->num_nodes();
  int total_elements = node_elements + elcs_wanted.size();
  
  DenseMatrixHandle output (boost::make_shared<DenseMatrix>(total_elements,1));
  int cnt = 0; 
  double module_half_done=total_elements*2; /// half of the runtime is reserved for rhs
  
  /// su
  for (int i=0; i < total_elements; i++)
  {
    if (i < node_elements)
      (*output)(i,0) = 0.0; // for the nodes
    else
      (*output)(i,0) = elcs_wanted[i-node_elements].toDouble()/1000.0; /// for electrodes ~ converting to Amps
    
    cnt++;
    if (cnt == total_elements/4)
    {
      cnt = 0;
      update_progress_max(i, module_half_done); /// progress bar is devided in 2 parts; first part = create rhs and second for lhs
    }
  }

  return output;
}

boost::tuple<DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle, FieldHandle, DenseMatrixHandle> SetupRHSforTDCSandTMSAlgorithm::run(FieldHandle mesh, const std::vector<Variable>& elcs, const std::vector<Variable>& impelc, int num_of_elc, FieldHandle scalp_tri_surf, FieldHandle elc_tri_surf, DenseMatrixHandle elc_sponge_location) const
{
  if (num_of_elc > 128) { THROW_ALGORITHM_INPUT_ERROR("Number of electrodes given exceeds what is possible ");}  /// number of possible electrodes is currently bound to 128 electrodes in default setting
  if (num_of_elc < 0) { THROW_ALGORITHM_INPUT_ERROR("Negative number of electrodes given ");}
  
  if (!mesh) THROW_ALGORITHM_INPUT_ERROR("Input field (mesh) was not allocated "); 
  if (!scalp_tri_surf) THROW_ALGORITHM_INPUT_ERROR("Input field (scalp triangle surface) was not allocated ");
  if (!elc_tri_surf) THROW_ALGORITHM_INPUT_ERROR("Input field (electrode triangle surface) was not allocated ");
  if (!elc_sponge_location) THROW_ALGORITHM_INPUT_ERROR("Input field (electrode triangle surface) was not allocated ");
  
  DenseMatrixHandle rhs=create_rhs(mesh, elcs, num_of_elc); /// get the right-hand-side
  
  DenseMatrixHandle lhs_knowns, elc_element, elc_element_typ, elc_element_def, elc_contact_imp;
  FieldHandle elec_sponge_surf;
  boost::tie(lhs_knowns, elc_element, elc_element_typ, elc_element_def, elc_contact_imp, elec_sponge_surf) = create_lhs(mesh, impelc,scalp_tri_surf, elc_tri_surf, elc_sponge_location); /// get various left-hand-side outputs
  
  DenseMatrixHandle selectmatrixind(new DenseMatrix(mesh->vmesh()->num_nodes(), 1)); ///create indeces for SelectSubMatrix
  for(long i=0;i<mesh->vmesh()->num_nodes();i++)
  { 
   (*selectmatrixind)(i,0)=i;
  }
  
  return boost::make_tuple(lhs_knowns, elc_element, elc_element_typ, elc_element_def, elc_contact_imp, rhs, elec_sponge_surf, selectmatrixind);
}
