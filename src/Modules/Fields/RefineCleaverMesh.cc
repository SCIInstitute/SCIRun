/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
/// @todo Documentation Modules/Legacy/Fields/RefineCleaverMesh.cc

#include <Core/Algorithms/Field/RefineCleaverMeshAlgorithm.h> 
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Scalar.h>
#include <Modules/Fields/RefineCleaverMesh.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

const ModuleLookupInfo RefineCleaverMesh::staticInfo_("RefineCleaverMesh", "ChangeMesh", "SCIRun"); 

RefineCleaverMesh::RefineCleaverMesh()
		:Module(staticInfo_, true)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputField);
}

void RefineCleaverMesh::setStateDefaults()
{
 setStateDoubleFromAlgo(Parameters::RefineCleaverMeshIsoValue);	
 setStateDoubleFromAlgo(Parameters::RefineCleaverMeshEdgeLength);	
 setStateDoubleFromAlgo(Parameters::RefineCleaverMeshVolume);	
 setStateDoubleFromAlgo(Parameters::RefineCleaverMeshDihedralAngleSmaller);
 setStateDoubleFromAlgo(Parameters::RefineCleaverMeshDihedralAngleBigger);
 setStateBoolFromAlgo(Parameters::RefineCleaverMeshDoNoSplitSurfaceTets);
 get_state()->setValue(Parameters::RefineCleaverMeshRadioButtons, 0);
}

void RefineCleaverMesh::execute()
{

 FieldHandle inputfield_ = getRequiredInput(InputField);

 auto tmp_matrix=getOptionalInput(InputMatrix);

 DenseMatrixHandle inputmatrix_; 

 if (tmp_matrix) 
    inputmatrix_ = matrix_cast::as_dense(*(tmp_matrix));

  if (needToExecute() )
  {
    update_state(Executing);
    setAlgoDoubleFromState(Parameters::RefineCleaverMeshIsoValue);
    setAlgoDoubleFromState(Parameters::RefineCleaverMeshEdgeLength);
    setAlgoDoubleFromState(Parameters::RefineCleaverMeshVolume);
    setAlgoDoubleFromState(Parameters::RefineCleaverMeshDihedralAngleSmaller);
    setAlgoDoubleFromState(Parameters::RefineCleaverMeshDihedralAngleBigger);
    setAlgoBoolFromState(Parameters::RefineCleaverMeshDoNoSplitSurfaceTets);
    auto RadioButton = get_state()->getValue(Parameters::RefineCleaverMeshRadioButtons).toInt();
    algo().set(Parameters::RefineCleaverMeshRadioButtons, RadioButton);      
    auto output = algo().run_generic(withInputData((InputField, inputfield_)(InputMatrix, inputmatrix_)));
    sendOutputFromAlgorithm(OutputField, output);
  }
}
