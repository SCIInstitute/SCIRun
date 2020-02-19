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
#include <Modules/Legacy/Fields/ConvertIndicesToFieldData.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <Dataflow/Network/Module.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms;

/// @class ConvertIndicesToFieldData
/// @brief Convert a field with indices as data values into a field with values
/// assigned to each index using a lookup table.

MODULE_INFO_DEF(ConvertIndicesToFieldData, ChangeFieldData, SCIRun)

ConvertIndicesToFieldData::ConvertIndicesToFieldData() : Module(staticInfo_)
{
	INITIALIZE_PORT(InputField);
	INITIALIZE_PORT(InputMatrix);
	INITIALIZE_PORT(OutputField);
}

void ConvertIndicesToFieldData::setStateDefaults()
{
	setStateStringFromAlgoOption(Parameters::OutputFieldDataType);
}

void
ConvertIndicesToFieldData::execute()
{
	auto input_field = getRequiredInput(InputField);
	auto input_matrix = getRequiredInput(InputMatrix);

	if (needToExecute())
	{
		setAlgoOptionFromState(Parameters::OutputFieldDataType);

		auto output = algo().run(withInputData((InputField, input_field)(InputMatrix, input_matrix)));

		sendOutputFromAlgorithm(OutputField, output);
  }
}
