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

#include <Modules/Legacy/Fields/ExtractSimpleIsosurface.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/ExtractSimpleIsosurfaceAlgo.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

ExtractSimpleIsosurface::ExtractSimpleIsosurface()
  : Module(ModuleLookupInfo("ExtractSimpleIsosurface", "NewField", "SCIRun"))
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(Isovalue);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(OutputMatrix);
}

void ExtractSimpleIsosurface::setStateDefaults()
{
  setStateDoubleFromAlgo(Parameters::SingleIsoValue);
  setStateStringFromAlgo(Parameters::ListOfIsovalues);
  setStateIntFromAlgo(Parameters::QuantityOfIsovalues);
  get_state()->setValue(Parameters::IsovalueListString, std::string());
  get_state()->setValue(Parameters::IsovalueChoice, std::string("Single"));
}

void ExtractSimpleIsosurface::execute()
{
  auto field = getRequiredInput(InputField);
  auto isovalueOption = getOptionalInput(Isovalue);

  if (needToExecute())
  {
    auto state = get_state();

    if (isovalueOption && *isovalueOption && !(*isovalueOption)->empty())
    {
      if (state->getValue(Parameters::IsovalueChoice).toString() == "Single")
      {
        double iso = (*isovalueOption)->get(0,0);
        state->setValue(Parameters::SingleIsoValue, iso);
      }
      else if (state->getValue(Parameters::IsovalueChoice).toString() == "List")
      {
        if (!matrixIs::dense(*isovalueOption)) error("Isovalue input matrix should be dense type");
        
        auto mat_iso = castMatrix::toDense (*isovalueOption);
        if (mat_iso->nrows()>1)
        {
          if (mat_iso->ncols()>1)
          {
            warning("input matrix will work better as a row matrix");
          }
          else
          {
            mat_iso->transposeInPlace();
          }
        }
        
        std::ostringstream ostr;
        ostr << *mat_iso;
        state->setValue(Parameters::ListOfIsovalues, ostr.str());
      }
    }

    std::vector<double> isoDoubles;

    if (state->getValue(Parameters::IsovalueChoice).toString() == "Single")
    {
      auto singleIso = state->getValue(Parameters::SingleIsoValue).toDouble();
      isoDoubles.push_back(singleIso);
    }
    else if (state->getValue(Parameters::IsovalueChoice).toString() == "List")
    {
      auto isoList = state->getValue(Parameters::ListOfIsovalues).toString();
      std::vector<std::string> tokens;
      boost::split(tokens, isoList, boost::is_any_of(", "));

      std::transform(tokens.begin(), tokens.end(), std::back_inserter(isoDoubles), [](const std::string& s)
      {
        try { return boost::lexical_cast<double>(s); } catch (boost::bad_lexical_cast&) { return 0.0; }
      });
    }
    else if (state->getValue(Parameters::IsovalueChoice).toString() == "Quantity")
    {
      //TODO: add exclusive/inclusive option; move to algo level
      double qmin, qmax;
      field->vfield()->minmax(qmin, qmax);
      std::ostringstream ostr;
      int num = state->getValue(Parameters::QuantityOfIsovalues).toInt();
      if (num > 1)
      {
        double di = (qmax - qmin) / (double)(num - 1.0);
        for (int i = 0; i < num; i++)
        {
          isoDoubles.push_back(qmin + ((double)i*di));
          ostr << isoDoubles[i] << "\n";
        }
      }
      else if (num == 1)
      {
        isoDoubles.push_back((qmin + qmax)/2);
        ostr << isoDoubles[0] << "\n";
      }
      state->setValue(Parameters::IsovalueListString, ostr.str());
    }

    VariableList isos;
    std::transform(isoDoubles.begin(), isoDoubles.end(), std::back_inserter(isos), [](double x) { return makeVariable("iso", x); });
    algo().set(Parameters::Isovalues, isos);
    
    auto output = algo().run(withInputData((InputField, field)));
    sendOutputFromAlgorithm(OutputField, output);
    sendOutputFromAlgorithm(OutputMatrix, output);
  }
}
