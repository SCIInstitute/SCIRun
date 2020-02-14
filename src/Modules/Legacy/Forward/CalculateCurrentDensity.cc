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


#include <Modules/Legacy/Forward/CalculateCurrentDensity.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Tensor.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Modules::Forward;
using namespace SCIRun::Dataflow::Networks;

MODULE_INFO_DEF(CalculateCurrentDensity, Forward, SCIRun)

CalculateCurrentDensity::CalculateCurrentDensity() : Module(staticInfo_,false)
{
  INITIALIZE_PORT(TetMesh_EField);
  INITIALIZE_PORT(TetMesh_Sigmas);
  INITIALIZE_PORT(Currents);
}

void CalculateCurrentDensity::execute()
{
  auto efieldH = getRequiredInput(TetMesh_EField);
  auto sigmasH = getRequiredInput(TetMesh_Sigmas);

  if (needToExecute())
  {
    VField* efield = efieldH->vfield();
    VMesh*  emesh = efieldH->vmesh();
    VField* sfield = sigmasH->vfield();

    if (sfield->basis_order() != 0)
    {
      error("Need sigmas at Elements");
      return;
    }

    if (efield->basis_order() != 0)
    {
      error("Need efield at Elements");
      return;
    }

    if (!efield->is_vector())
    {
      error("Electric field needs to vector data");
      return;
    }

    if (emesh->num_elems() != sigmasH->vmesh()->num_elems())
    {
      error("Electric field have same number of elements as sigma field");
      return;
    }

    FieldInformation eInfo(efieldH);
    eInfo.make_vector();
    FieldHandle output_field = CreateField(eInfo, efieldH->mesh());
    VField* ofield = output_field->vfield();

    VMesh::size_type num_elems = emesh->num_elems();

    Vector vec;
    Vector e;
    for (VMesh::Elem::index_type idx = 0; idx < num_elems; idx++)
    {
      efield->get_value(e, idx);

      Tensor s;
      sfield->get_value(s, idx);

      // - sign added to vector to account for E = - Del V
      vec = Vector(-(s.val(0, 0) * e.x() + s.val(0, 1) * e.y() + s.val(0, 2) * e.z()),
                   -(s.val(1, 0) * e.x() + s.val(1, 1) * e.y() + s.val(1, 2) * e.z()),
                   -(s.val(2, 0) * e.x() + s.val(2, 1) * e.y() + s.val(2, 2) * e.z()));

      ofield->set_value(vec, idx);
    }

    sendOutput(Currents, output_field);
  }
}
