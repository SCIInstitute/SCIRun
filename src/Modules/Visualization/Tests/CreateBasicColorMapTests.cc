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
*/

#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Core/Datatypes/ColorMap.h>
#include <Modules/Visualization/CreateBasicColorMap.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Utils/Exception.h>

using namespace SCIRun::Testing;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core;

class CreateBasicColorMapModuleTest : public ModuleTest
{
};

TEST_F(CreateBasicColorMapModuleTest, ThrowsForUnknownColorMapName)
{
  UseRealModuleStateFactory f;

  auto cbc = makeModule("CreateStandardColorMap");

  cbc->get_state()->setValue(Variables::ColorMapName, std::string("null"));

  EXPECT_THROW(cbc->execute(), InvalidArgumentException);
}

TEST_F(CreateBasicColorMapModuleTest, DISABLED_CreatesRainbowByDefault)
{
  UseRealModuleStateFactory f;

  auto cbc = makeModule("CreateStandardColorMap");

  cbc->execute();

  FAIL() << "TODO: need output value collector dummy module: See Issue #499";
}