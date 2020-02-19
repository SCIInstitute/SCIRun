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


///
///@file  GenerateLinearSegments.cc
///@brief Unfinished modules
///
///@author
///   Michael Callahan
///   Department of Computer Science
///   University of Utah
///@date  March 2001
///

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/FieldInformation.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>
#include <Core/Containers/Handle.h>

#include <sstream>

namespace SCIRun {

class GenerateLinearSegments : public Module
{
public:
  GenerateLinearSegments(GuiContext* ctx);
  virtual ~GenerateLinearSegments() {}

  virtual void execute();

private:
  GuiDouble                     gui_step_size_;
  GuiInt                        gui_max_steps_;
  GuiInt                        gui_direction_;
  GuiInt                        gui_value_;
};


DECLARE_MAKER(GenerateLinearSegments)

GenerateLinearSegments::GenerateLinearSegments(GuiContext* ctx)
  : Module("GenerateLinearSegments", ctx, Filter, "MiscField", "SCIRun"),
  gui_step_size_(get_ctx()->subVar("stepsize"), 0.1),
  gui_max_steps_(get_ctx()->subVar("maxsteps"), 10),
  gui_direction_(get_ctx()->subVar("direction"), 1),
  gui_value_(get_ctx()->subVar("value"), 1)
{
}

void
GenerateLinearSegments::execute()
{
  FieldHandle field_input_handle;

  get_input_handle( "Input Field", field_input_handle, true );



  // Check to see if the input field(s) has changed.
  if( inputs_changed_ ||
      gui_step_size_.changed( true ) ||
      gui_max_steps_.changed( true ) ||
      gui_direction_.changed( true ) ||
      gui_value_.changed( true ) ||
      !oport_cached("Output  Linear Segments") )
  {
    update_state(Executing);

    if (!(field_input_handle->vfield()->is_vector()))
    {
      error( "This module only works on vector data.");
      return;
    }

    if (!(field_input_handle->vfield()->is_lineardata()))
    {
      error( "This module only works for data on the nodes of the mesh.");
      return;
    }

    VField* ifield = field_input_handle->vfield();
    VMesh*  imesh  = field_input_handle->vmesh();

    std::string datatype;
    FieldInformation fi("CurveMesh",1,"double");
    if (gui_value_.get() == 0) fi.make_vector();
    FieldHandle field_output_handle = CreateField(fi);


    VField* ofield = field_output_handle->vfield();
    VMesh*  omesh  = field_output_handle->vmesh();

    int step_size = gui_step_size_.get();
		int max_steps = gui_max_steps_.get();
    int direction = gui_direction_.get();
    int value = gui_value_.get();

    Point seed;
    Vector vec;
    std::vector<Point> nodes;
    VMesh::Node::array_type edge(2);
    nodes.reserve(direction==1?2*max_steps:max_steps);

    VMesh::size_type num_nodes = imesh->num_nodes();

    std::vector<Point>::iterator node_iter;
    VMesh::Node::index_type n1, n2;

    int cnt = 0;
    for(VMesh::Node::index_type idx=0; idx < num_nodes; idx++)
    {
      imesh->get_center(seed, idx);
      ifield->get_value(vec, idx);

      cnt++; if (cnt == 100) { cnt = 0; update_progress(cnt, num_nodes); }

      nodes.clear();
      nodes.push_back(seed);

      size_t cc = 0;

      // Find the negative segments.
      if( direction <= 1 )
      {
        for (int i=1; i < max_steps; i++)
        {
          nodes.push_back(seed+(double)i*step_size*(-vec));
        }
        if ( direction == 1 )
        {
          std::reverse(nodes.begin(), nodes.end());
          cc = nodes.size();
          cc = -(cc - 1);
        }
      }
      // Append the positive segments.
      if( direction >= 1 )
      {
        for (int i=1; i < max_steps; i++)
        {
          nodes.push_back(seed+(double)i*step_size*vec);
        }
      }

      node_iter = nodes.begin();

      if (node_iter != nodes.end())
      {
        n1 = omesh->add_node(*node_iter);

        std::ostringstream str;
        str << "Segment " << idx << " Node Index";
        ofield->set_property( str.str(), static_cast<index_type>(n1), false );

        ofield->resize_values();

        if (value == 0)
        {
          ofield->copy_value(ifield,idx,n1);
        }
        else if( value == 1)
        {
          ofield->set_value(idx,n1);
        }
        else if (value == 2)
        {
          ofield->set_value(fabs((double)cc),n1);
        }

        ++node_iter;

        cc++;

        while (node_iter != nodes.end())
        {
          n2 = omesh->add_node(*node_iter);
          ofield->resize_values();

          if (value == 0)
          {
             ofield->copy_value(ifield,idx,n2);
          }
          else if( value == 1)
          {
            ofield->set_value(idx,n2);
          }
          else if (value == 2)
          {
            ofield->set_value(fabs((double)cc),n2);
          }

          edge[0] = n1;
          edge[1] = n2;
          omesh->add_elem(edge);

          n1 = n2;
          ++node_iter;
          cc++;
        }
      }
    }

    ofield->set_property( "Segment Count", num_nodes, false );
    send_output_handle( "Output Linear Segments", field_output_handle );
  }
}

} // End namespace SCIRun
