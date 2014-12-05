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

#include <Modules/Legacy/Fields/InterfaceWithTetGen.h>
#include <Modules/Legacy/Fields/InterfaceWithTetGenImpl.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Dataflow/Network/ModuleStateInterface.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

const ModuleLookupInfo InterfaceWithTetGen::staticInfo_("InterfaceWithTetGen", "NewField", "SCIRun");

InterfaceWithTetGen::InterfaceWithTetGen()
  : Module(staticInfo_)
{
  INITIALIZE_PORT(Main);
  INITIALIZE_PORT(Points);
  INITIALIZE_PORT(Region_Attribs);
  INITIALIZE_PORT(Regions);
  INITIALIZE_PORT(TetVol);
}

void InterfaceWithTetGen::setStateDefaults()
{
  //TODO
}

void InterfaceWithTetGen::execute()
{
  auto first_surface = getRequiredInput(Main);
  auto rest = getOptionalDynamicInputs(Regions);
  std::deque<FieldHandle> surfaces(rest.begin(), rest.end());
  surfaces.push_front(first_surface);

  auto points = getOptionalInput(Points);
  auto region_attribs = getOptionalInput(Region_Attribs);









  //TODO
}
