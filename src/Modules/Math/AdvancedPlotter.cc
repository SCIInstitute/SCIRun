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


#include <Modules/Math/BasicPlotter.h>
#include <Modules/Math/AdvancedPlotter.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

MODULE_INFO_DEF(LinePlotter, Math, SCIRun);

ALGORITHM_PARAMETER_DEF(Math, IndependentVariablesVector);
ALGORITHM_PARAMETER_DEF(Math, DependentVariablesVector);

LinePlotter::LinePlotter() : Module(staticInfo_)
{
  INITIALIZE_PORT(IndependentVariable);
  INITIALIZE_PORT(DependentVariables);
}

void LinePlotter::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::PlotTitle, std::string("Plot title"));
  state->setValue(Parameters::DataTitle, std::string("Data title"));
  state->setValue(Parameters::XAxisLabel, std::string("x axis"));
  state->setValue(Parameters::YAxisLabel, std::string("y axis"));
  state->setValue(Parameters::VerticalAxisVisible, true);
  state->setValue(Parameters::HorizontalAxisVisible, true);
  state->setValue(Parameters::VerticalAxisPosition, 0.0);
  state->setValue(Parameters::HorizontalAxisPosition, 0.0);
  state->setValue(Parameters::ShowPointSymbols, true);
  state->setValue(Parameters::CurveStyle, std::string("Lines"));
  auto colors = makeAnonymousVariableList(
    ColorRGB(0x27213cu).toString(),
    ColorRGB(0x5A352Au).toString(),
    ColorRGB(0xA33B20u).toString(),
    ColorRGB(0xA47963u).toString(),
    ColorRGB(0xA6A57Au).toString()
  );
  state->setValue(Parameters::PlotColors, colors);
  state->setValue(Parameters::PlotBackgroundColor, std::string());
  state->setValue(Parameters::TransposeData, false);
}

void LinePlotter::execute()
{
  auto independents = getOptionalDynamicInputs(IndependentVariable);
  auto dependents = getOptionalDynamicInputs(DependentVariables);

  if (needToExecute())
  {
    if (dependents.empty())
    {
      error("Empty matrix input: dependent variable data");
      return;
    }
    if (independents.size() < dependents.size())
    {
      remark("Independent variable input insufficient: using row index of dependent data as independent variable.");
      for (int m = independents.size(); m < dependents.size(); ++m)
      {
        auto rowCount = dependents[m]->nrows();
        auto indexMatrix(boost::make_shared<DenseMatrix>(rowCount, 1));
        for (int i = 0; i < rowCount; ++i)
          (*indexMatrix)(i, 0) = i;
        independents.push_back(indexMatrix);
      }
    }
    else if (independents.size() > dependents.size())
    {
      independents.resize(dependents.size());
      remark("Ignoring unmatched independent variable matrices.");
    }
    for (int i = 0; i < independents.size(); ++i)
    {
      auto expectedRows = independents[i]->nrows();
      auto actualRows = dependents[i]->nrows();
      if (actualRows != expectedRows)
      {
        std::ostringstream ostr;
        ostr << "Dependent variable matrix size inconsistency. Input matrices at index " << i << " have unequal row counts: independent matrix has " << expectedRows
          << " but dependent matrix has " << actualRows << ".";
        error(ostr.str());
        return;
      }

      if (independents[i]->ncols() != dependents[i]->ncols())
      {
        std::ostringstream ostr;
        ostr << "Due to different numbers of columns in input pair " << i << ", all dependent data columns will be plotted against the first independent data column for that input.";
        warning(ostr.str());
      }
    }

    get_state()->setTransientValue(Parameters::IndependentVariablesVector, independents);
    get_state()->setTransientValue(Parameters::DependentVariablesVector, dependents);
  }
}
