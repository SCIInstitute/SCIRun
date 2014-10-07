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

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(BrainStimulator, ElectrodeTableValues);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ELECTRODE_VALUES);

AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::refnode() { return AlgorithmParameterName("refnode"); }

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

AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::ElecrodeParameterName(int i) { return AlgorithmParameterName(Name("elc"+boost::lexical_cast<std::string>(i)));}

SetupRHSforTDCSandTMSAlgorithm::SetupRHSforTDCSandTMSAlgorithm()
{
  addParameter(Parameters::ELECTRODE_VALUES, 0); // just a default value, will be replaced with vector
  addParameter(refnode(), 0);
}

AlgorithmOutput SetupRHSforTDCSandTMSAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto mesh = input.get<Field>(MESH);
  
  // obtaining electrode values from the state
  auto all_elc_values = get(Parameters::ELECTRODE_VALUES).getList();
  
  // consistency check:
  for (int i=0; i<all_elc_values.size(); i++)
  {
    auto elecName = all_elc_values[i].name_; 
   // auto elecValue = all_elc_values[i].getDouble(); // why is not that used?
    auto expectedElecName = SetupRHSforTDCSandTMSAlgorithm::ElecrodeParameterName(i); // ElecrodeParameterName(i);
    if(elecName.name_.compare(expectedElecName.name_) != 0) // if so, electrodes are being stored out of order.
      THROW_ALGORITHM_PROCESSING_ERROR("Values are being stored out of order!");
  }
  
  auto scalp_tri_surf = input.get<Field>(SCALP_TRI_SURF_MESH);
  auto elc_tri_surf = input.get<Field>(ELECTRODE_TRI_SURF_MESH);
  
  // obtaining number of electrodes
  DenseMatrixHandle elc_sponge_location = matrix_convert::to_dense(input.get<Matrix>(ELECTRODE_SPONGE_LOCATION_AVR));
  if (!elc_sponge_location)
  {
   THROW_ALGORITHM_PROCESSING_ERROR("Electrode sponges matrix (center locations) is not allocated."); 
  }
  if (elc_sponge_location->ncols()!=3 || elc_sponge_location->nrows()<2)
  {
   THROW_ALGORITHM_PROCESSING_ERROR("Electrode sponges matrix needs to have dimension #sponges x 3 (#sponges>=2)"); 
  }

  int num_of_elc = elc_sponge_location->nrows();
  
  AlgorithmOutput output; 
  
  DenseMatrixHandle lhs_knowns, elc_element, elc_element_typ, elc_element_def, elc_contact_imp, rhs;
  boost::tie(lhs_knowns, elc_element, elc_element_typ, elc_element_def, elc_contact_imp, rhs) = run(mesh, all_elc_values, num_of_elc, scalp_tri_surf, elc_tri_surf, elc_sponge_location);

  output[LHS_KNOWNS] = lhs_knowns;
  output[ELECTRODE_ELEMENT] = elc_element;
  output[ELECTRODE_ELEMENT_TYPE] = elc_element_typ;
  output[ELECTRODE_ELEMENT_DEFINITION] = elc_element_def;
  output[ELECTRODE_CONTACT_IMPEDANCE] = elc_contact_imp;
  output[RHS] = rhs;
  return output;
}

/// replace this code with calls to splitfieldbyconnectedregion, clipfieldby* if available for SCIRun5 
boost::tuple<DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle> SetupRHSforTDCSandTMSAlgorithm::create_lhs(FieldHandle mesh, FieldHandle scalp_tri_surf, FieldHandle elc_tri_surf, DenseMatrixHandle elc_sponge_location) const
{
 VMesh::size_type mesh_num_nodes = mesh->vmesh()->num_nodes();
 DenseMatrixHandle lhs_knows, elc_elem, elc_elem_typ, elc_elem_def, elc_con_imp;
 
 index_type refnode_number = get(refnode()).toInt();
 if ( refnode_number > mesh_num_nodes && refnode_number>0)
 {
    THROW_ALGORITHM_PROCESSING_ERROR(" Reference node exceeds number of FEM nodes. ");
 }
 
 /// prepare LHS_KNOWNS which is the an input to addknownstolinearsystem (called x) besides the stiffness matrix
 lhs_knows=boost::make_shared<DenseMatrix>(mesh_num_nodes,1);
 for(VMesh::Node::index_type idx=0; idx<mesh_num_nodes; idx++)
 {
   (*lhs_knows)(idx,1)=std::numeric_limits<double>::quiet_NaN();
 }
 (*lhs_knows)(refnode_number,1)=0;
 
 SplitFieldByConnectedRegionAlgo algo;
 algo.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), false);
 algo.set(SplitFieldByConnectedRegionAlgo::SortAscending(), false);
 std::vector<FieldHandle> result = algo.run(mesh);
 
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
 
  /// map the electrode sponge center (CreateElectrodeCoil) to generated tDCS electrode geometry (Cleaver), 
  /// this mapping is meant to map the GUI inputs to actual electrode geometry by having a lookup table
 DenseMatrixHandle lookup(new DenseMatrix(elc_sponge_location->nrows(), 1));
 DenseMatrixHandle distances(new DenseMatrix(elc_sponge_location->nrows(), 1));
 VMesh::Node::index_type didx;
 double distance=0; 
 for (long i=0;i<elc_sponge_location->nrows();i++)
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
 
 /// throw error if the geometry is too far away from predicted location (based on input)
 for (long i=0;i<elc_sponge_location->nrows();i++)
 {
   if ((*distances)(i,0)>(*elc_sponge_location)(i,3))
   {
    std::ostringstream ostr1;
    ostr1 << " distance to electrode " << i << " sponge  = " <<  (*distances)(i,0) << " exceeds defined limit of " << (*elc_sponge_location)(i,3) << std::endl;
    THROW_ALGORITHM_PROCESSING_ERROR(ostr1.str());
   }
 }
 
 /// determine intersection of electrode sponge and scalp (replace with a mesh intersection function later); as well as center of electrode sponge/scalp surface
 FieldInformation fi("PointCloudMesh",0,"double");
 FieldHandle intersection_points=CreateField(fi);
 VMesh* mesh_intersection_points = intersection_points->vmesh();
 VField* field_intersection_points = intersection_points->vfield();
 VMesh* mesh_scalp_tri_surf  = scalp_tri_surf->vmesh(); 
 Point p; Vector norm; 
 std::vector<VMesh::Node::index_type> node_ind;
 std::vector<double> dist; 
 distance=std::numeric_limits<double>::infinity(); 
 DenseMatrixHandle elctrode_sponge_geometry_location_avr(new DenseMatrix(result.size(), 3));
 DenseMatrixHandle sponge_center_pojected_onto_scalp(new DenseMatrix(result.size(), 3));
 for(long i=0;i<result.size();i++)
 {
  VMesh*  tmp_mesh = result[i]->vmesh();
  tmp_mesh->synchronize(Mesh::NODE_LOCATE_E);
  double x=0,y=0,z=0,count=0;
  long number_electrode_nodes=tmp_mesh->num_nodes();
  for (VMesh::Node::index_type idx=0; idx<number_electrode_nodes; idx++)
  {
    tmp_mesh->get_center(p,idx);
    if (mesh_scalp_tri_surf->find_closest_nodes(dist, node_ind, p, identical_node_location_differce))
    {
     if(node_ind.size()==1)
      {
       mesh_intersection_points->add_point(p);
       tmp_mesh->get_normal(norm,idx);
       field_intersection_points->set_value(norm,idx);
       x+=p.x(); 
       y+=p.y(); 
       z+=p.z();
       count++;
      } else
      {
       THROW_ALGORITHM_PROCESSING_ERROR(" Found multiple points. Decrease minimum distance bound. ");
      }
    }
  }
  Point r(x/count,y/count,z/count);
  (*elctrode_sponge_geometry_location_avr)(i,0)=r.x();
  (*elctrode_sponge_geometry_location_avr)(i,1)=r.y();
  (*elctrode_sponge_geometry_location_avr)(i,2)=r.z();
   mesh_intersection_points->find_closest_node(distance,p,didx,r);
  (*sponge_center_pojected_onto_scalp)(i,0)=p.x();
  (*sponge_center_pojected_onto_scalp)(i,1)=p.y();
  (*sponge_center_pojected_onto_scalp)(i,2)=p.z();
  
  ///move from the center of the electrode sponge to the top by going in the opposite direction of sponge_center_pojected_onto_scalp using the normal
  /// first decide what the opposite direction is
  
  
 }
 
 /// find center of scalp-elc intersection

 
  //FieldInformation fi(result[i]);
  //FieldHandle tmp_field=CreateField(fi); /// create intersection surface to be plugged into "intersection_elcsponge_scalp"   
  //VField* ofield = tmp_field->vfield();
  //ofield->resize_values(); 
  //VMesh::Node::index_type nodeindex = clipped->add_point(np);
  //clipped->add_elem(nnodes);
 
 
/*
 VMesh*   elc_mesh = elc_tri_surf->vmesh();
 Point p,r;
 double distance;
 VMesh::Node::index_type didx;
 DenseMatrixHandle elc_scalp_tri_surf_locations = DenseMatrixHandle(new DenseMatrix(num_nodes, 3));
 DenseMatrixHandle remaining_elc_scalp_tri_surf_locations = DenseMatrixHandle(new DenseMatrix(num_nodes, 3));
 
 VMesh*  scalp_mesh = scalp_tri_surf->vmesh();
 scalp_mesh->synchronize(Mesh::NODE_LOCATE_E);
 
 long elc_scalp_tri_surf_locations_count=0, remaining_elc_scalp_tri_surf_locations_count=0;
 /// find the intersections of scalp-electrode sponges meshes and mark those for further exclusion
 for(VMesh::Node::index_type idx=0; idx<num_nodes; idx++)
 {
  elc_mesh->get_center(p,idx);
  scalp_mesh->find_closest_node(distance,r,didx,p);
  if (distance<=identical_node_location_differce) 
  {
    (*elc_scalp_tri_surf_locations)(elc_scalp_tri_surf_locations_count,0)=p.x();    
    (*elc_scalp_tri_surf_locations)(elc_scalp_tri_surf_locations_count,1)=p.y(); 
    (*elc_scalp_tri_surf_locations)(elc_scalp_tri_surf_locations_count,2)=p.z(); 
    elc_scalp_tri_surf_locations_count++;
  } else
  {
    (*remaining_elc_scalp_tri_surf_locations)(remaining_elc_scalp_tri_surf_locations_count,0)=r.x();
    (*remaining_elc_scalp_tri_surf_locations)(remaining_elc_scalp_tri_surf_locations_count,1)=r.y();
    (*remaining_elc_scalp_tri_surf_locations)(remaining_elc_scalp_tri_surf_locations_count,2)=r.z();
    remaining_elc_scalp_tri_surf_locations_count++;
  }
 }
 /// find the location on electrode sponge surface and project it to the top surface based on scalp-sponge normal if needed
 for (long i=0; i<elc_sponge_location->ncols(); i++)
 {
  double x=(*elc_sponge_location)(i,0), y=(*elc_sponge_location)(i,1), z=(*elc_sponge_location)(i,2);
  double min_dist_sponge=std::numeric_limits<double>::max(), min_dist_scalp=std::numeric_limits<double>::max();
  double x1=0,y1=0,z1=0;
  long index_sponge=-1;
  for (long j=0; j<elc_scalp_tri_surf_locations_count; j++)
  {
    x1=(*elc_scalp_tri_surf_locations)(j,0);
    y1=(*elc_scalp_tri_surf_locations)(j,1);
    z1=(*elc_scalp_tri_surf_locations)(j,2);
    distance = sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1)+(z-z1)*(z-z1));
    if (min_dist_sponge>distance)
    {
     index_sponge=j;
     min_dist_sponge=distance;
    }
  }
  
  if (distance>(*elc_sponge_location)(i,4))
   {
    THROW_ALGORITHM_PROCESSING_ERROR("Averaged electrode sponge location seems to be outside of the electrode sponge");   
   } else
   {
    /// is it projected on the sponge surface or on the scalp or the sides of the sponge -> test it!
    /// 1) test if it is on scalp or sponge sides
    long index_elc_scalp_interface=-1;
    for (long j=0; j<elc_scalp_tri_surf_locations_count; j++)
    { 
     x1=(*elc_scalp_tri_surf_locations)(j,0);
     y1=(*elc_scalp_tri_surf_locations)(j,1);
     z1=(*elc_scalp_tri_surf_locations)(j,2);
     distance = sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1)+(z-z1)*(z-z1));
     if (min_dist_scalp>distance)
     {
      index_elc_scalp_interface=j;
      min_dist_scalp=distance;
     }
    }
   }
 } 
 */
 /*
 scalp_mesh->synchronize(Mesh::NODE_LOCATE_E); 
 std::vector<double> distances;
 std::vector<VMesh::Node::index_type> nodes;
 const Point q;
 double maxdist = std::numeric_limits<double>::max();
 /// map the input electrode locations on the scalp-electrode interface
 for (long i=0; i<elc_sponge_location->nrows(); i++)
 {
  p=Point((*elc_sponge_location)(i,0),(*elc_sponge_location)(i,1),(*elc_sponge_location)(i,2));
  scalp_mesh-> find_closest_nodes(distances, nodes, q,  maxdist);
  for (int j=0; j<distances.size(); j++)
    std::cout << "j: " << j << distances[j] << std::endl;
 }*/
 
 /*
 VMesh*  imesh  = mesh->vmesh();
 double min=std::numeric_limits<double>::max();
 
 /// 2) identify the intersection using the scalp mesh
 for(VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
 {
  imesh->get_center(p,idx);
  distance = sqrt((x1-p.x())*(x1-p.x())+(y1-p.y())*(y1-p.y())+(z1-p.z())*(z1-p.z()));
 
 }
 
 
   Vector norm;
  for (VMesh::Node::index_type i=0; i<num_nodes; ++i)
  {
    vmesh->get_normal(norm,i);
    dataptr[k] = norm.x();
    dataptr[k+1] = norm.y();
    dataptr[k+2] = norm.z();
 
 */
 
/*
 VMesh::size_type numnodes = imesh->num_nodes();    
    VMesh::index_type count = 0;    
    for (VMesh::Node::index_type i=0; i< numnodes; i++)
    {
      Point p;
      imesh->get_center(p,i);
      VMesh::Elem::index_type eidx;
      if (objmesh->locate(eidx,p))
      {
        // it is inside the volume
        omesh->add_point(p);
      }
      else
      {
        Point r;
        smesh->find_closest_elem(r,eidx,p);
        omesh->add_point(r);
      }
      count++; if (count == 100) { update_progress(i,numnodes); count=0; }
    }*/


 return boost::make_tuple(lhs_knows, elc_elem, elc_elem_typ, elc_elem_def, elc_con_imp);
}

DenseMatrixHandle SetupRHSforTDCSandTMSAlgorithm::create_rhs(FieldHandle mesh, const std::vector<Variable>& elcs, int num_of_elc) const
{
 // storing only desired amount of electrodes to pass to run method
  std::vector<Variable, std::allocator<Variable>> elcs_wanted; 
  for (int i=0; i<num_of_elc; i++)
    elcs_wanted.push_back(elcs[i]);

  VField* vfield = mesh->vfield();
 
  // making sure current magnitudes of the electrodes summed are greater than 10e-6
  double min_current = 0;
  for (int i=0; i<elcs_wanted.size(); i++)
  {
    double temp = std::fabs(elcs_wanted[i].toDouble());
    min_current += temp;
  }
  if (min_current < 0.00001) remark("Electrode current intensities are negligible");
  
  int node_elements  = vfield->vmesh()->num_nodes();
  int total_elements = node_elements + elcs_wanted.size();
  
  DenseMatrixHandle output (boost::make_shared<DenseMatrix>(total_elements,1));
  int cnt = 0; 
  double module_half_done=total_elements*2;
  for (int i=0; i < total_elements; i++)
  {
    if (i < node_elements)
      (*output)(i,0) = 0.0; // for the nodes
    else
      (*output)(i,0) = elcs_wanted[i-node_elements].toDouble()/1000.0; // for electrodes ~ converting to Amps
    
    cnt++;
    if (cnt == total_elements/4)
    {
      cnt = 0;
      update_progress_max(i, module_half_done); /// progress bar is devided in 2 parts; first part = create rhs and second for lhs
    }
  }

  return output;
}

boost::tuple<DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle, DenseMatrixHandle> SetupRHSforTDCSandTMSAlgorithm::run(FieldHandle mesh, const std::vector<Variable>& elcs, int num_of_elc, FieldHandle scalp_tri_surf, FieldHandle elc_tri_surf, DenseMatrixHandle elc_sponge_location) const
{
  if (num_of_elc > 128) { THROW_ALGORITHM_INPUT_ERROR("Number of electrodes given exceeds what is possible ");}
  else if (num_of_elc < 0) { THROW_ALGORITHM_INPUT_ERROR("Negative number of electrodes given ");}
  
  if (!mesh) THROW_ALGORITHM_INPUT_ERROR("Input field (mesh) was not allocated "); 
  if (!scalp_tri_surf) THROW_ALGORITHM_INPUT_ERROR("Input field (scalp triangle surface) was not allocated ");
  if (!elc_tri_surf) THROW_ALGORITHM_INPUT_ERROR("Input field (electrode triangle surface) was not allocated ");
  if (!elc_sponge_location) THROW_ALGORITHM_INPUT_ERROR("Input field (electrode triangle surface) was not allocated ");
  
  DenseMatrixHandle rhs=create_rhs(mesh, elcs, num_of_elc);
  
  DenseMatrixHandle lhs_knowns, elc_element, elc_element_typ, elc_element_def, elc_contact_imp;
  boost::tie(lhs_knowns, elc_element, elc_element_typ, elc_element_def, elc_contact_imp) = create_lhs(mesh, scalp_tri_surf, elc_tri_surf, elc_sponge_location); 
  
  return boost::make_tuple(lhs_knowns, elc_element, elc_element_typ, elc_element_def, elc_contact_imp, rhs);
}
