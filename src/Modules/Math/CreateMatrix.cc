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


/// @todo Documentation Modules/Math/CreateMatrix.cc

#include <algorithm>
#include <map>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Modules/Math/CreateMatrix.h>
#include <Core/Datatypes/MatrixIO.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

MODULE_INFO_DEF(CreateMatrix, Math, SCIRun)

ALGORITHM_PARAMETER_DEF(Math, TextEntry);
ALGORITHM_PARAMETER_DEF(Math, MatrixTextDelimiter);

CreateMatrix::CreateMatrix() : Module(staticInfo_)
{
  INITIALIZE_PORT(EnteredMatrix);
}

void CreateMatrix::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Core::Algorithms::Math::Parameters::TextEntry, std::string());
  state->setValue(Core::Algorithms::Math::Parameters::MatrixTextDelimiter, std::string("Space"));
}

void CreateMatrix::execute()
{
  if (needToExecute())
  {
    auto matrix(makeShared<DenseMatrix>());
    try
    {
      auto state = get_state();
      auto matrixString = state->getValue(Core::Algorithms::Math::Parameters::TextEntry).toString();

      if (!matrixString.empty())
      {
        // The stream parser splits values on whitespace, so a non-space
        // delimiter is normalized to spaces first. Unknown or empty delimiter
        // names (e.g. networks saved before this option existed) fall through
        // to the historical space-delimited behavior.
        static const std::map<std::string, char> delimiters =
          {{"Space", ' '}, {"Comma", ','}, {"Semicolon", ';'}, {"Tab", '\t'}};
        const auto delimiterName = state->getValue(Core::Algorithms::Math::Parameters::MatrixTextDelimiter).toString();
        const auto delimiter = delimiters.find(delimiterName);
        if (delimiter != delimiters.end() && delimiter->second != ' ')
          std::replace(matrixString.begin(), matrixString.end(), delimiter->second, ' ');

        matrixString += "\n";
        std::istringstream reader(matrixString);

        reader >> *matrix;
      }
    }
    catch (...)
    {
      THROW_ALGORITHM_INPUT_ERROR("Matrix parsing failed.");
    }
    sendOutput(EnteredMatrix, matrix);
  }
}
