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



SetupRHSforTDCSandTMSAlgorithm::SetupRHSforTDCSandTMSAlgorithm()
{
  
}

AlgorithmOutput SetupRHSforTDCSandTMSAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto elc_coil_pos_and_normal = input.get<Field>(ELECTRODE_COIL_POSITIONS_AND_NORMAL);
  auto elc_count               = input.get<Matrix>(ELECTRODE_COUNT);

  //! try:
  auto elc_values = get(Parameters::ELECTRODE_VALUES).getList();
  for (int i=0; i<elc_values.size(); i++)
  {
    std::cout << elc_values[i].name_ << "=" << elc_values[i].value_ << std::endl;
  //  auto elecName = electrodes[i].name_;
  //  auto elecValue = electrodes[i].getDouble();
  //  // need a consistency check:
  //  auto expectedElecName = electrodeName(i);
  //  EXPECT_EQ(elecName, expectedElecName); // if not, electrodes are being stored out of order. 
  //  //You may not care about the name--if so just ignore the above.
  }
  
  // obtaining number of electrodes
  DenseMatrixHandle elc_count_dense (new DenseMatrix(matrix_cast::as_dense(elc_count)->block(0,0,elc_count->nrows(),elc_count->ncols()))); 
  int num_of_elc = elc_count_dense->coeff(0,0);
  
  // building the output rhs
  AlgorithmOutput output;
  DenseMatrixHandle rhs = run(elc_coil_pos_and_normal, num_of_elc);
  output[RHS] = rhs;
  return output;

//  auto tri = input.get<Field>(ELECTRODE_TRIANGULATION);
//  auto tri2 = input.get<Field>(ELECTRODE_TRIANGULATION2);
//  auto coil = input.get<Field>(COIL);
//  auto coil2 = input.get<Field>(COIL2);
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(pos_orient, "ELECTRODE_COIL_POSITIONS_AND_NORMAL input field");
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri, "ELECTRODE_TRIANGULATION input field");
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri2, "ELECTRODE_TRIANGULATION2 input field");
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil, "COIL input field");
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil2, "COIL2 input field");
//  output[ELECTRODES_FIELD] = out1;
//  output[COILS_FIELD] = out2;
}

DenseMatrixHandle SetupRHSforTDCSandTMSAlgorithm::run(FieldHandle fh, int num_of_elc) const
{  
  if (num_of_elc > 128) { THROW_ALGORITHM_INPUT_ERROR("Number of electrodes given exceeds what is possible ");}
  else if (num_of_elc < 0) { THROW_ALGORITHM_INPUT_ERROR("Negative number of electrodes given ");}
  
  if (!fh) THROW_ALGORITHM_INPUT_ERROR("Input field was not allocated ");
  
  VField* vfield = fh->vfield();
 
  // making sure current intensities of the electrodes together are greater than 10e-6
  double check_value = 0;
  for (int i=0; i<num_of_elc; i++)
  {
    double temp = 10.0;// TODO electrode_values[i]/1000.0;
    if (temp < 0.0)
      temp = temp*(-1.0);
    check_value += temp;
  }
  if (check_value < 0.00001) THROW_ALGORITHM_INPUT_ERROR("Electrode current intensities are negligible ");
  
  int node_elements  = vfield->vmesh()->num_nodes();
  int total_elements = node_elements + num_of_elc;
  
  DenseMatrixHandle output (boost::make_shared<DenseMatrix>(total_elements,1));
  int cnt = 0;
  for (int i=0; i < total_elements; i++)
  {
    if (i < node_elements)
      (*output)(i,0) = 0.0;
    else
      (*output)(i,0) = 5.0; // TODO electrode_values[i-node_elements]/1000.0; // converting to Amps
    
    cnt++;
    if (cnt == total_elements/4)
    {
      cnt = 0;
      update_progress((double)i/total_elements);
    }
  }
  return output;
}
