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


#include <Core/Algorithms/Legacy/Fields/FieldData/ConvertIndicesToFieldDataAlgo.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms::Fields::Parameters;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(Fields, OutputFieldDataType);

ConvertIndicesToFieldDataAlgo::ConvertIndicesToFieldDataAlgo()
{
  addOption(Parameters::OutputFieldDataType, "double","double|float|char|unsigned char|short|unsigned short|int|unsigned int");
}

bool
ConvertIndicesToFieldDataAlgo::runImpl(FieldHandle input_field, DenseMatrixHandle input_matrix, FieldHandle& output_field) const
{
  ScopedAlgorithmStatusReporter r(this, "ConvertIndicesToFieldData");

  if (!input_field)
  {
    error("No input field");
    return (false);
  }

  FieldInformation fi(input_field);
  output_field = CreateField(fi);
	FieldInformation fo(output_field);

	if (fi.is_nonlinear())
	{
		error("This function has not yet been defined for non-linear elements");
		return (false);
	}

	if (fi.is_nodata())
	{
		error("This function has not yet been defined for fields with no data");
		return (false);
	}

	if (fi.is_vector() || fi.is_tensor())
	{
		error("This function has not yet been defined for fields with vectors or tensors as indices");
		return (false);
	}

 	size_type nrows = input_matrix->nrows();
	size_type ncols = input_matrix->ncols();

	std::string algotype;

	if (ncols == 1)
	{
		algotype = "Scalar";
	}
	else if (ncols == 3)
	{
		algotype = "Vector";
	}
	else if (ncols == 6 || ncols == 9)
	{
		algotype = "Tensor";
	}
	else
	{
		if (nrows == 1)
		{
			algotype = "Scalar";
		}
		else if (nrows == 3)
		{
			algotype = "Vector";
		}
		else if (nrows == 6 || nrows == 9)
		{
			algotype = "Tensor";
		}
		else
		{
			error("Data does not have dimension of 1, 3, 6, or 9");
			return (false);
		}
	}

	if (algotype == "Scalar")
	{
		std::string datatype;
		getOption(Parameters::OutputFieldDataType);
		fo.set_data_type(getOption(Parameters::OutputFieldDataType));
	}
	if (algotype == "Vector") fo.make_vector();
	if (algotype == "Tensor") fo.make_tensor();

	//--------------------------------
	// VIRTUAL INTERFACE

	output_field = CreateField(fo, input_field->mesh());
	VField* vinput = input_field->vfield();
	VField* voutput = output_field->vfield();
	voutput->resize_fdata();

	if (algotype == "Scalar")
	{
		int max_index = input_matrix->nrows() * input_matrix->ncols();
		const double *dataptr = input_matrix->data();
		VMesh::size_type sz = vinput->num_values();
		for (VMesh::index_type r = 0; r<sz; r++)
		{
			int idx;
			vinput->get_value(idx, r);
			if ((idx < 0) || (idx >= max_index))
			{
				error("Index exceeds matrix dimensions");
				return (false);
			}
			voutput->set_value(dataptr[idx], r);
		}
		return (true);
	}
	else if (algotype == "Vector")
	{
		if (input_matrix->ncols() != 3)
		{
			input_matrix.reset(new DenseMatrix(input_matrix->transpose()));
		}

		const double *dataptr = input_matrix->data();
		int max_index = input_matrix->nrows();

		VMesh::size_type sz = vinput->num_values();
		for (VMesh::index_type r = 0; r<sz; r++)
		{
			int idx;
			vinput->get_value(idx, r);
			if ((idx < 0) || (idx >= max_index))
			{
				error("Index exceeds matrix dimensions");
				return (false);
			}
			voutput->set_value(Vector(dataptr[3 * idx], dataptr[3 * idx + 1], dataptr[3 * idx + 2]), r);
		}
		return (true);
	}
	else if (algotype == "Tensor")
	{
		if ((input_matrix->ncols() != 6) && (input_matrix->ncols() != 9))
		{
			input_matrix.reset(new DenseMatrix(input_matrix->transpose()));
		}

		int max_index = input_matrix->nrows();
		const double *dataptr = input_matrix->data();
		int ncols = input_matrix->ncols();

		VMesh::size_type sz = vinput->num_values();
		for (VMesh::index_type r = 0; r<sz; r++)
		{
			int idx;
			vinput->get_value(idx, r);
			if ((idx < 0) || (idx >= max_index))
			{
				error("Index exceeds matrix dimensions");
				return (false);
			}
			if (ncols == 6)
			{
				voutput->set_value(Tensor(dataptr[3 * idx], dataptr[3 * idx + 1], dataptr[3 * idx + 2], dataptr[3 * idx + 3], dataptr[3 * idx + 4], dataptr[3 * idx + 5]), r);
			}
			else
			{
				voutput->set_value(Tensor(dataptr[3 * idx], dataptr[3 * idx + 1], dataptr[3 * idx + 2], dataptr[3 * idx + 4], dataptr[3 * idx + 5], dataptr[3 * idx + 8]), r);
			}
		}
		AlgorithmOutput output;
		output[Variables::OutputField] = output_field;
		return (true);
	}

	// keep the compiler happy:
	// it seems reasonable to return false if none of the cases apply (AK)
	return (false);
}

AlgorithmOutput ConvertIndicesToFieldDataAlgo::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);
  auto inputmatrix = input.get<DenseMatrix>(Variables::InputMatrix);

  FieldHandle output_field;
  if (!runImpl(field, inputmatrix, output_field))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = output_field;
  return output;
}
