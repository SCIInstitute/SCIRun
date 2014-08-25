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

   Author: Spencer Frisby
   Date:   July 2014
*/

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/BrainStimulator/SetupRHSforTDCSandTMSAlgorithm.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <boost/lexical_cast.hpp>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(BrainStimulator, ElectrodeTableValues);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ELECTRODE_VALUES);

AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::MESH("MESH");
AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_COUNT("ELECTRODE_COUNT");
AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::SCALP_TRI_SURF_MESH("SCALP_TRI_SURF_MESH");
AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_TRI_SURF_MESH("ELECTRODE_TRI_SURF_MESH");
AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_SPONGE_LOCATION_AVR("ELECTRODE_SPONGE_LOCATION_AVR");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_ELEMENT("ELECTRODE_ELEMENT");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_ELEMENT_TYPE("ELECTRODE_ELEMENT_TYPE");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_ELEMENT_DEFINITION("ELECTRODE_ELEMENT_DEFINITION");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_CONTACT_IMPEDANCE("ELECTRODE_CONTACT_IMPEDANCE");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::RHS("RHS");

AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::ElecrodeParameterName(int i) { return AlgorithmParameterName(Name("elc"+boost::lexical_cast<std::string>(i)));}

SetupRHSforTDCSandTMSAlgorithm::SetupRHSforTDCSandTMSAlgorithm()
{
  addParameter(Parameters::ELECTRODE_VALUES, 0); // just a default value, will be replaced with vector
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
  
  // obtaining number of electrodes
  auto elc_count = input.get<Matrix>(ELECTRODE_COUNT);
  DenseMatrixHandle elc_count_dense (new DenseMatrix(matrix_cast::as_dense(elc_count)->block(0,0,elc_count->nrows(),elc_count->ncols())));
  int num_of_elc = elc_count_dense->coeff(0,0);
  
  auto scalp_tri_surf = input.get<Field>(SCALP_TRI_SURF_MESH);
  auto elc_tri_surf = input.get<Field>(ELECTRODE_TRI_SURF_MESH);
  
  DenseMatrixHandle elc_sponge_location = matrix_convert::to_dense(input.get<Matrix>(ELECTRODE_SPONGE_LOCATION_AVR));
  // making the rhs, sending it back as output
  AlgorithmOutput output; 
  
  DenseMatrixHandle rhs = run(mesh, all_elc_values, num_of_elc, scalp_tri_surf, elc_tri_surf, elc_sponge_location);

  output[RHS] = rhs;
  return output;
}

boost::tuple<DenseMatrixHandle> SetupRHSforTDCSandTMSAlgorithm::create_lhs(FieldHandle mesh, FieldHandle elc_tri_surf, DenseMatrixHandle elc_sponge_location) const
{
 DenseMatrixHandle output;
 
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


 return output;
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
    double temp = std::fabs(elcs_wanted[i].getDouble());
    min_current += temp;
  }
  if (min_current < 0.00001) remark("Electrode current intensities are negligible");
  
  int node_elements  = vfield->vmesh()->num_nodes();
  int total_elements = node_elements + elcs_wanted.size();
  
  DenseMatrixHandle output (boost::make_shared<DenseMatrix>(total_elements,1));
  int cnt = 0;
  for (int i=0; i < total_elements; i++)
  {
    if (i < node_elements)
      (*output)(i,0) = 0.0; // for the nodes
    else
      (*output)(i,0) = elcs_wanted[i-node_elements].getDouble()/1000.0; // for electrodes ~ converting to Amps
    
    cnt++;
    if (cnt == total_elements/4)
    {
      cnt = 0;
      update_progress_max(i, total_elements*2); /// progress bar is devided in 2 parts; first part = create rhs and second for lhs
    }
  }

  return output;
}

DenseMatrixHandle SetupRHSforTDCSandTMSAlgorithm::run(FieldHandle mesh, const std::vector<Variable>& elcs, int num_of_elc, FieldHandle scalp_tri_surf, FieldHandle elc_tri_surf, DenseMatrixHandle elc_sponge_location) const
{
  if (num_of_elc > 128) { THROW_ALGORITHM_INPUT_ERROR("Number of electrodes given exceeds what is possible ");}
  else if (num_of_elc < 0) { THROW_ALGORITHM_INPUT_ERROR("Negative number of electrodes given ");}
  
  if (!mesh) THROW_ALGORITHM_INPUT_ERROR("Input field was not allocated ");
  
  DenseMatrixHandle rhs=create_rhs(mesh, elcs, num_of_elc);
  //DenseMatrixHandle lhs=create_lhs(scalp_tri_surf, elc_tri_surf, elc_sponge_location);
  
  DenseMatrixHandle output;
  
  return output;
}
