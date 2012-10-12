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

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

/*
 *  CreateLatVol.cc:  Make an ImageField that fits the source field.
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   March 2001
 *
 */

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Datatypes/Matrix.h>

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Util/StringUtil.h>

#include <Dataflow/GuiInterface/GuiVar.h>

#include <iostream>

DECLARE_MAKER(CreateLatVol)

CreateLatVol::CreateLatVol(GuiContext* ctx)
  : Module("CreateLatVol", ctx, Filter, "NewField", "SCIRun"),
    size_x_(get_ctx()->subVar("sizex"), 16),
    size_y_(get_ctx()->subVar("sizey"), 16),
    size_z_(get_ctx()->subVar("sizez"), 16),
    padpercent_(get_ctx()->subVar("padpercent"), 0.0),
    data_at_(get_ctx()->subVar("data-at"), "Nodes"),
    element_size_(get_ctx()->subVar("element-size"),"Mesh")
{
}


void
CreateLatVol::execute()
{
  FieldHandle   ifieldhandle;
  MatrixHandle  Size;
	
  get_input_handle("Input Field", ifieldhandle, false);
  get_input_handle("LatVol Size",Size,false);
	
  if (inputs_changed_ || size_x_.changed() || size_y_.changed() ||
      size_z_.changed() || padpercent_.changed() || data_at_.changed() ||
      element_size_.changed() || !oport_cached("Output Sample Field") )
  {
    update_state(Executing);
    
    Point minb, maxb;
    DataTypeEnum datatype;

    if (Size.get_rep())
    {
      if (Size->get_data_size() == 1)
      {
        double* data = Size->get_data_pointer();
        unsigned int size = static_cast<unsigned int>(data[0]);
        size_x_.set(size);
        size_y_.set(size);
        size_z_.set(size);
        get_ctx()->reset();
      }
      else if (Size->get_data_size() == 3)
      {
        double* data = Size->get_data_pointer();
        unsigned int size1 = static_cast<unsigned int>(data[0]);
        unsigned int size2 = static_cast<unsigned int>(data[1]);
        unsigned int size3 = static_cast<unsigned int>(data[2]);
        size_x_.set(size1);
        size_y_.set(size2);
        size_z_.set(size3);		
        get_ctx()->reset();		
      }
      else
      {
        error("LatVol size matrix needs to have or one element or three elements");
      }	
    }	
		
    // Create blank mesh.
    VField::size_type sizex = Max(2, size_x_.get());
    VField::size_type sizey = Max(2, size_y_.get());
    VField::size_type sizez = Max(2, size_z_.get());		
		
    if (ifieldhandle.get_rep() == 0)
    {
      datatype = SCALAR;
      if (element_size_.get() == "Mesh")
      {
        minb = Point(-1.0, -1.0, -1.0);
        maxb = Point(1.0, 1.0, 1.0);
      }
      else
      {
        minb = Point(0.0,0.0,0.0);
        maxb = Point(static_cast<double>(sizex-1),
                     static_cast<double>(sizey-1),
                     static_cast<double>(sizez-1));
      }
    }
    else
    {
      datatype = SCALAR;
      FieldInformation fi(ifieldhandle);
      if (fi.is_vector())
      {
        datatype = VECTOR;
      }
      else if (fi.is_tensor())
      {
        datatype = TENSOR;
      }
      BBox bbox = ifieldhandle->vmesh()->get_bounding_box();
      minb = bbox.min();
      maxb = bbox.max();
    }

    Vector diag((maxb.asVector() - minb.asVector()) * (padpercent_.get()/100.0));
    minb -= diag;
    maxb += diag;

    int basis_order;
    if (data_at_.get() == "Nodes") basis_order = 1;
    else if (data_at_.get() == "Cells") basis_order = 0;
    else if (data_at_.get() == "None") basis_order = -1;
    else 
    {
      error("Unsupported data_at location " + data_at_.get() + ".");
      return;
    }

    FieldInformation lfi("LatVolMesh",basis_order,"double");
    if (datatype == VECTOR) lfi.make_vector();
    else if (datatype == TENSOR) lfi.make_tensor();

    // Create Image Field.
    MeshHandle mesh = CreateMesh(lfi,sizex, sizey, sizez, minb, maxb);
    FieldHandle ofh = CreateField(lfi,mesh);
    ofh->vfield()->clear_all_values();

    send_output_handle("Output Sample Field", ofh);
  }
}


} // End namespace SCIRun

