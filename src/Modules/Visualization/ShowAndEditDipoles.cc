/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Modules/Visualization/ShowAndEditDipoles.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Core;
using namespace Core::Datatypes;
using namespace Algorithms;

MODULE_INFO_DEF(ShowAndEditDipoles, Visualization, SCIRun)

ShowAndEditDipoles::ShowAndEditDipoles() : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(OutputGeom);
}

void ShowAndEditDipoles::execute()
{
  auto field = getRequiredInput(InputField);
  if(needToExecute())
    {
      std::cout << "executing\n";
      FieldInformation finfo(field);
      if(!finfo.is_pointcloudmesh())
        {
          error("Input field was not a valid point cloud.");
        }
    }
}

void ShowAndEditDipoles::setStateDefaults()
{
  auto state = get_state();

  state->setValue(Sizing, 0);
  state->setValue(ShowLastAsVector, false);
  state->setValue(ShowLines, false);
}

const AlgorithmParameterName ShowAndEditDipoles::Sizing("Sizing");
const AlgorithmParameterName ShowAndEditDipoles::ShowLastAsVector("ShowLastAsVector");
const AlgorithmParameterName ShowAndEditDipoles::ShowLines("ShowLines");
