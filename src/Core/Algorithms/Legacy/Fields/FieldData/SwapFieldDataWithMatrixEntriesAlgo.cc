/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Legacy/Fields/FieldData/SwapFieldDataWithMatrixEntriesAlgo.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>
#include <Core/Datatypes/Legacy/Matrix/MatrixTypeConverter.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h> 

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms::Fields::Parameters; 
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(Fields, PreserveScalar)

SwapFieldDataWithMatrixEntriesAlgo::SwapFieldDataWithMatrixEntriesAlgo()
{
  addParameter(PreserveScalar, false);
}

bool
SwapFieldDataWithMatrixEntriesAlgo::runImpl(FieldHandle input_field, MatrixHandle input_matrix, FieldHandle& output_field, MatrixHandle& output_matrix) const
{
  ScopedAlgorithmStatusReporter r(this, "SwapFieldDataWithMatrixEntriesAlgo");

  if (!input_field)
  {
    error("No input field");
    return (false);
  }
  
  FieldInformation fo(input_field);
  
  const bool preserve_scalar = get(Parameters::PreserveScalar).toBool();
	if( output_matrix )
    {
      MatrixHandle matrix_output_handle;
      if(!(get_algo_.run(input_field))) 
			{
					matrix_output_handle = get_algo_.run(input_field); 
					return;
			}
			output_matrix = matrix_output_handle; 
      //send_output_handle("Output Matrix", matrix_output_handle);  
    }

    // Set the data.
    if( output_field )
    {
      FieldHandle field_output_handle;

      if (input_matrix)//because smart pointers?  
      {
        if (preserve_scalar) 
          set_algo_.set_option("scalardatatype",input_field->vfield()->get_data_type()); 
					//set_algo_.setscalardata(field_input_handle, input_matrix,); 
					//set_algo_.set_option("scalardatatype", input_field->vfield()); 
        if(!(set_algo_.run(input_field, input_matrix, output_field))) return;

        field_output_handle->copy_properties(input_field);
      }
      else 
      {
        warning("No input matrix passing the field through");
        output_field = input_field;
      }	

      AlgorithmOutput output;
			output[Variables::OutputField] = output_field;
    }
  return (true);
}

bool
SwapFieldDataWithMatrixEntriesAlgo::runImpl(FieldHandle input, FieldHandle& output) const
{
  MatrixHandle dummy;
	MatrixHandle dummy2; 
  return runImpl(input, dummy, output, dummy2);
}

AlgorithmOutput SwapFieldDataWithMatrixEntriesAlgo::run_generic(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);

  FieldHandle outputField;
  if (!runImpl(field, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;
  return output;
}
