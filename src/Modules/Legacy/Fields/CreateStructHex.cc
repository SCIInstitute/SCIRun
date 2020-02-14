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


///@author
///   Michael Callahan
///   Department of Computer Science
///   University of Utah
///@date  March 2001

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Util/StringUtil.h>
#include <iostream>

namespace SCIRun {

/// @class CreateStructHex
/// @brief This module constructs a StructHexVolField that fits the source field.

class CreateStructHex : public Module
{
  public:
    CreateStructHex(GuiContext* ctx);
    virtual ~CreateStructHex() {}

    virtual void execute();

  private:
    GuiInt size_x_;
    GuiInt size_y_;
    GuiInt size_z_;
    GuiDouble padpercent_;
    GuiString data_at_;

    enum DataTypeEnum { SCALAR, VECTOR, TENSOR };
};


DECLARE_MAKER(CreateStructHex)

CreateStructHex::CreateStructHex(GuiContext* ctx)
  : Module("CreateStructHex", ctx, Filter, "NewField", "SCIRun"),
    size_x_(get_ctx()->subVar("sizex"), 16),
    size_y_(get_ctx()->subVar("sizey"), 16),
    size_z_(get_ctx()->subVar("sizez"), 16),
    padpercent_(get_ctx()->subVar("padpercent"), 0.0),
    data_at_(get_ctx()->subVar("data-at"), "Nodes")
{
}


void
CreateStructHex::execute()
{
  FieldHandle ifieldhandle;
  Point minb, maxb;
  DataTypeEnum datatype;
  if (!get_input_handle("Input Field", ifieldhandle, false))
  {
    datatype = SCALAR;
    minb = Point(-1.0, -1.0, -1.0);
    maxb = Point(1.0, 1.0, 1.0);
  }
  else
  {
    datatype = SCALAR;
    if (ifieldhandle->vfield()->is_vector())
    {
      datatype = VECTOR;
    }
    else if (ifieldhandle->vfield()->is_tensor())
    {
      datatype = TENSOR;
    }
    BBox bbox = ifieldhandle->vmesh()->get_bounding_box();
    minb = bbox.min();
    maxb = bbox.max();
  }

  update_state(Executing);

  Vector diag((maxb.asVector() - minb.asVector()) * (padpercent_.get()/100.0));
  minb -= diag;
  maxb += diag;

  // Create blank mesh.
  unsigned int sizex = Max(2, size_x_.get());
  unsigned int sizey = Max(2, size_y_.get());
  unsigned int sizez = Max(2, size_z_.get());


  int basis_order;
  if (data_at_.get() == "Nodes") basis_order = 1;
  else if (data_at_.get() == "Cells") basis_order = 0;
  else if (data_at_.get() == "None") basis_order = -1;
  else {
    error("Unsupported data_at location " + data_at_.get() + ".");
    return;
  }

  FieldInformation sfi("StructHexVolMesh",basis_order,"double");
  if (datatype == VECTOR) sfi.make_vector();
  if (datatype == TENSOR) sfi.make_tensor();

  MeshHandle mesh = CreateMesh(sfi,sizex, sizey, sizez);
  VMesh* vmesh = mesh->vmesh();

  Transform trans;
  trans.pre_scale(Vector(1.0 / (sizex-1.0),
			 1.0 / (sizey-1.0),
			 1.0 / (sizez-1.0)));
  trans.pre_scale(maxb - minb);
  trans.pre_translate(minb.asVector());

  VMesh::Node::iterator mitr, mitr_end;
  vmesh->begin(mitr);
  vmesh->end(mitr_end);

  VMesh::index_type i,j,k;
  while (mitr != mitr_end)
  {
    vmesh->from_index(i,j,k,*mitr);
    const Point p0(static_cast<double>(i),
     static_cast<double>(j), static_cast<double>(k));
    const Point p = trans.project(p0);
    vmesh->set_point(p, *mitr);
    ++mitr;
  }
  vmesh->set_transform(trans);

  // Create Image Field.
  FieldHandle ofh = CreateField(sfi,mesh);
  ofh->vfield()->clear_all_values();

  send_output_handle("Output Sample Field", ofh);
}


} // End namespace SCIRun
