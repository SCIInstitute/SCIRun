/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2020 Scientific Computing and Imaging Institute,
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

#include <Modules/Visualization/CreateTestingArrow.h>
#include <Graphics/Widgets/WidgetFactory.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Graphics::Datatypes;

MODULE_INFO_DEF(CreateTestingArrow, Visualization, SCIRun)

CreateTestingArrow::CreateTestingArrow() : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(Arrow);
}

void CreateTestingArrow::setStateDefaults()
{
  //auto state = get_state();
}

void CreateTestingArrow::execute()
{
  auto arrow = WidgetFactory::createArrowWidget(
    {*this, "testArrow1"},
    {
      {10.0, "", {0,1,2}, {{0,0,0}, {1,1,1}}, 10},
      {1,1,0}, {2,2,0}, true, 2, 4
    }
  );
  sendOutput(Arrow, arrow);
}
