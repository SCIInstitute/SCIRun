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


#include <Modules/Legacy/Fields/CalculateNormals.h>
#include <Core/Datatypes/String.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

/// @class CalculateNormals
/// @brief Make a new vector field that points to the input point.

MODULE_INFO_DEF(CalculateNormals, ChangeFieldData, SCIRun) ;

CalculateNormals::CalculateNormals() : Module(staticInfo_, false)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(InputPoint);
  INITIALIZE_PORT(OutputField);
}

void
CalculateNormals::execute()
{
  auto ifieldhandle = getRequiredInput(InputField);
  auto ipointhandle = getRequiredInput(InputPoint);

  if ( inputs_changed_ || !oport_cached("Output Field"))
  {
    update_state(Executing);

    VField* point_field = ipointhandle->vfield();
    VMesh*  point_mesh  = ipointhandle->vmesh();

    if (point_mesh->num_nodes() != 1)
    {
      error("Input Point needs to have a single node only");
      return;
    }

    Point attract_point;
    Vector dir;
    bool has_vector = false;
    point_mesh->get_center(attract_point,VMesh::Node::index_type(0));

    if (!(point_field->is_pointcloudmesh()))
    {
      error("Input field was not a valid point cloud.");
      return;
    }

    if (point_field->is_vector())
    {
      has_vector = true;
      point_field->get_value(dir,0);
    }

    VField* field = ifieldhandle->vfield();

    VField::size_type num_values = field->num_values();

    FieldInformation fi(ifieldhandle);
    if (fi.field_basis_order() < 1)
    {
      fi.make_lineardata();
      fi.make_vector();
    }

    fi.make_vector();

    auto ofieldhandle = CreateField(fi,ifieldhandle->mesh());
    VField* ofield = ofieldhandle->vfield();
    ofield->resize_values();

    int cnt = 0;

    if (field->is_scalar() && field->basis_order() == 1)
    {
      Point c; Vector vec; Vector diff; double val;
      for (VField::index_type idx = 0; idx<num_values;idx++)
      {
        field->get_center(c,idx);
        if (has_vector)
        {
           diff = c - attract_point;
           vec = (dir*Dot(dir,diff)-diff);
           vec.safe_normalize();
        }
        else
        {
          vec = (attract_point - c);
          vec.safe_normalize();
        }

        field->get_value(val,idx);
        if (val == 0.0) val = 1e-3;
        vec = vec*val;
        ofield->set_value(vec,idx);
        cnt++; if (cnt == 400) { cnt=0; update_progress(idx,num_values); }
      }
    }
    else
    {
      Point c; Vector vec; Vector diff;
      for (VField::index_type idx = 0; idx<num_values;idx++)
      {
        field->get_center(c,idx);
        if (has_vector)
        {
           diff = c - attract_point;
           vec = (dir*Dot(dir,diff)-diff);
           vec.safe_normalize();
        }
        else
        {
          vec = (attract_point - c);
          vec.safe_normalize();
        }
        ofield->set_value(vec,idx);
        cnt++; if (cnt == 400) { cnt=0; update_progress(idx,num_values); }
      }
    }

    sendOutput(OutputField, ofield);
  }
}
