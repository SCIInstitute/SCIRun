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

AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_COIL_POSITIONS_AND_NORMAL("ELECTRODE_COIL_POSITIONS_AND_NORMAL");
AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_COUNT("ELECTRODE_COUNT");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::RHS("RHS");
//const AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_TRIANGULATION("ELECTRODE_TRIANGULATION");
//const AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_TRIANGULATION2("ELECTRODE_TRIANGULATION2");
//const AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::COIL("COIL");
//const AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::COIL2("COIL2");
//const AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODES_FIELD("ELECTRODES_FIELD");
//const AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::COILS_FIELD("COILS_FIELD");


AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::ElecrodeParameterName(int i) { return AlgorithmParameterName(Name("elc"+boost::lexical_cast<std::string>(i)));}

SetupRHSforTDCSandTMSAlgorithm::SetupRHSforTDCSandTMSAlgorithm()
{
  addParameter(Parameters::ELECTRODE_VALUES, 0); // just a default value, will be replaced with vector
}

AlgorithmOutput SetupRHSforTDCSandTMSAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto elc_coil_pos_and_normal = input.get<Field>(ELECTRODE_COIL_POSITIONS_AND_NORMAL);
  
  // obtaining electrode values from the state
  auto all_elc_values = get(Parameters::ELECTRODE_VALUES).getList();
  
  // consistency check:
  for (int i=0; i<all_elc_values.size(); i++)
  {
    std::cout << all_elc_values[i].name_ << "=" << all_elc_values[i].value_ << std::endl;
    auto elecName = all_elc_values[i].name_; 
    auto elecValue = all_elc_values[i].getDouble();
    auto expectedElecName = SetupRHSforTDCSandTMSAlgorithm::ElecrodeParameterName(i); //ElecrodeParameterName(i);
    if(elecName.name_.compare(expectedElecName.name_) != 0) // if so, electrodes are being stored out of order.
      THROW_ALGORITHM_PROCESSING_ERROR("Values are being stored out of order!");
  }
  
  // obtaining number of electrodes
  auto elc_count = input.get<Matrix>(ELECTRODE_COUNT);
  DenseMatrixHandle elc_count_dense (new DenseMatrix(matrix_cast::as_dense(elc_count)->block(0,0,elc_count->nrows(),elc_count->ncols())));
  int num_of_elc = elc_count_dense->coeff(0,0);
  
  // making the rhs, sending it back as output
  AlgorithmOutput output;
  DenseMatrixHandle rhs = run(elc_coil_pos_and_normal, all_elc_values, num_of_elc);

  output[RHS] = rhs;
  return output;
}

DenseMatrixHandle SetupRHSforTDCSandTMSAlgorithm::run(FieldHandle fh, std::vector<Variable, std::allocator<Variable>> elcs, int num_of_elc) const
{
  if (num_of_elc > 128) { THROW_ALGORITHM_INPUT_ERROR("Number of electrodes given exceeds what is possible ");}
  else if (num_of_elc < 0) { THROW_ALGORITHM_INPUT_ERROR("Negative number of electrodes given ");}
  
  if (!fh) THROW_ALGORITHM_INPUT_ERROR("Input field was not allocated ");
  
  // storing only desired amount of electrodes to pass to run method
  std::vector<Variable, std::allocator<Variable>> elcs_wanted; 
  for (int i=0; i<num_of_elc; i++)
    elcs_wanted.push_back(elcs[i]);

  VField* vfield = fh->vfield();
 
  // making sure current magnitudes of the electrodes summed are greater than 10e-6
  double min_current = 0;
  for (int i=0; i<elcs_wanted.size(); i++)
  {
    double temp = elcs_wanted[i].getDouble();
    if (temp < 0.0)
      temp = temp*(-1.0);
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
      update_progress((double)i/total_elements);
    }
  }
  return output;
}
