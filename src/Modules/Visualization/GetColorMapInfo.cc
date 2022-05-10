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


#include <Modules/Visualization/GetColorMapInfo.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/String.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Core::Datatypes;
using namespace Dataflow::Networks;

MODULE_INFO_DEF(GetColorMapInfo, Visualization, SCIRun)

GetColorMapInfo::GetColorMapInfo() : Module(staticInfo_)
{
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(Description);
}

void GetColorMapInfo::setStateDefaults()
{
  // auto state = get_state();
  // state->setValue(DisplaySide, 0);
  // state->setValue(DisplayLength, 0);
  // state->setValue(TextSize, 8);
  // state->setValue(TextRed, 1.);
  // state->setValue(TextGreen, 1.);
  // state->setValue(TextBlue, 1.);
  // state->setValue(Labels, 10);
  // state->setValue(Scale, 1.0);
  // state->setValue(Units, std::string(""));
  // state->setValue(SignificantDigits, 2);
  // state->setValue(AddExtraSpace, false);
  // state->setValue(XTranslation, 0);
  // state->setValue(YTranslation, 0);
  // state->setValue(ColorMapName, std::string(""));
}

void GetColorMapInfo::execute()
{
  auto colorMap = getRequiredInput(ColorMapObject);
  if (needToExecute())
  {
    std::ostringstream ostr;
    ostr << id() << "$" <<
      colorMap->getColorMapInvert() << colorMap->getColorMapName() << colorMap->getColorMapRescaleScale() <<
      colorMap->getColorMapRescaleShift() << colorMap->getColorMapResolution() << colorMap.get() <<
      colorMap->getColorMapShift();

    sendOutput(Description, makeShared<String>(ostr.str()));
  }
}
