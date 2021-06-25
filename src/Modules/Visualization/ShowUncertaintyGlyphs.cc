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

#include <Modules/Visualization/ShowUncertaintyGlyphs.h>
#include <Core/Algorithms/Visualization/ShowUncertaintyGlyphsAlgorithm.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace Core::Datatypes;
using namespace Modules::Visualization;
using namespace Core::Algorithms;
using namespace Core::Algorithms::Visualization;

MODULE_INFO_DEF(ShowUncertaintyGlyphs, Visualization, SCIRun);

namespace SCIRun {
namespace Modules {
namespace Visualization {
ShowUncertaintyGlyphs::ShowUncertaintyGlyphs() : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputGeom);
}

void ShowUncertaintyGlyphs::setStateDefaults()
{
  auto state = get_state();
}

void ShowUncertaintyGlyphs::execute()
{
  auto mean = getRequiredInput(InputField);
  auto covariance = getRequiredInput(InputMatrix);

  if(needToExecute())
  {
    auto algorithm = ShowUncertaintyGlyphsAlgorithm();
    AlgorithmInput input = withInputData((InputField, mean)(InputMatrix, covariance));
    // AlgorithmInput input = withInputData((MeanTensorField, mean));
    auto output = algorithm.run(*this, input);
    sendOutputFromAlgorithm(OutputGeom, output);
  }
}

}}}

const AlgorithmParameterName ShowUncertaintyGlyphs::ShowTensorTab("ShowTensorTab");
const AlgorithmParameterName ShowUncertaintyGlyphs::ShowTensors("ShowTensors");
// const AlgorithmParameterName ShowFieldGlyphs::TensorsDisplayType("TensorsDisplayType");
const AlgorithmParameterName ShowUncertaintyGlyphs::TensorsColoring("TensorsColoring");
const AlgorithmParameterName ShowUncertaintyGlyphs::TensorsColoringDataInput("TensorsColoringDataInput");
const AlgorithmParameterName ShowUncertaintyGlyphs::TensorsTransparency("TensorsTransparency");
const AlgorithmParameterName ShowUncertaintyGlyphs::TensorsUniformTransparencyValue("TensorsUniformTransparencyValue");
const AlgorithmParameterName ShowUncertaintyGlyphs::SuperquadricEmphasis("SuperquadricEmphasis");
const AlgorithmParameterName ShowUncertaintyGlyphs::NormalizeTensors("NormalizeTensors");
const AlgorithmParameterName ShowUncertaintyGlyphs::TensorsScale("TensorsScale");
const AlgorithmParameterName ShowUncertaintyGlyphs::RenderTensorsBelowThreshold("RenderTensorsBelowThreshold");
const AlgorithmParameterName ShowUncertaintyGlyphs::TensorsThreshold("TensorsThreshold");
const AlgorithmParameterName ShowUncertaintyGlyphs::TensorsResolution("TensorsResolution");
