/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

//    File   : ExtractIsosurface.cc
//    Author : Yarden Livnat
//    Date   : Fri Jun 15 16:38:02 2001


#include <Core/Algorithms/Fields/MarchingCubes/MarchingCubes.h>
#include <Core/Util/StringUtil.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/ColorMapPort.h>
#include <Dataflow/Network/Ports/GeometryPort.h>
#include <Dataflow/Network/Module.h>

//#include <Core/Algorithms/Visualization/Noise.h>

#include <Dataflow/Modules/Visualization/share.h>

namespace SCIRun {

class SCISHARE ExtractIsosurface : public Module {

public:
  ExtractIsosurface(GuiContext* ctx);
  virtual ~ExtractIsosurface() {}
  virtual void execute();
  virtual void tcl_command(GuiArgs&, void*);

private:

  //! GUI variables
  GuiDouble  gui_iso_value_min_;
  GuiDouble  gui_iso_value_max_;
  GuiDouble  gui_iso_value_;
  GuiDouble  gui_iso_value_typed_;
  GuiInt     gui_iso_value_quantity_;
  GuiString  gui_iso_quantity_range_;
  GuiString  gui_iso_quantity_clusive_;
  GuiDouble  gui_iso_quantity_min_;
  GuiDouble  gui_iso_quantity_max_;
  GuiString  gui_iso_quantity_list_;
  GuiString  gui_iso_value_list_;
  GuiString  gui_iso_matrix_list_;
  GuiInt     gui_build_field_;
  GuiInt     gui_build_geom_;
  GuiInt     gui_transparency_;
  GuiString  gui_active_isoval_selection_tab_;
  GuiString  gui_active_tab_; 
  //gui_update_type_ must be declared after gui_iso_value_max_ which is
  //traced in the tcl code. If gui_update_type_ is set to Auto having it
  //last will prevent the net from executing when it is instantiated.
  GuiString  gui_update_type_;

  GuiDouble  gui_color_r_;
  GuiDouble  gui_color_g_;
  GuiDouble  gui_color_b_;
  GuiDouble  gui_color_a_;

  GuiInt     gui_num_threads_;
  //! status variables
  std::vector< double > isovals_;
  
  bool default_color_changed_;

  // Algortihms
  SCIRunAlgo::MarchingCubesAlgo marchingcubes_;
};

DECLARE_MAKER(ExtractIsosurface)

ExtractIsosurface::ExtractIsosurface(GuiContext* context) : 
  Module("ExtractIsosurface", context, Filter, "Visualization", "SCIRun"), 
  gui_iso_value_min_(context->subVar("isoval-min"),  0.0),
  gui_iso_value_max_(context->subVar("isoval-max"), 99.0),
  gui_iso_value_(context->subVar("isoval"), 0.0),
  gui_iso_value_typed_(context->subVar("isoval-typed"), 0.0),
  gui_iso_value_quantity_(context->subVar("isoval-quantity"), 1),
  gui_iso_quantity_range_(context->subVar("quantity-range"), "field"),
  gui_iso_quantity_clusive_(context->subVar("quantity-clusive"), "exclusive"),
  gui_iso_quantity_min_(context->subVar("quantity-min"),   0),
  gui_iso_quantity_max_(context->subVar("quantity-max"), 100),
  gui_iso_quantity_list_(context->subVar("quantity-list"), ""),
  gui_iso_value_list_(context->subVar("isoval-list"), "No values present."),
  gui_iso_matrix_list_(context->subVar("matrix-list"),
		       "No matrix present - execution needed."),
  gui_build_field_(context->subVar("build_trisurf"), 1),
  gui_build_geom_(context->subVar("build_geom"), 1),
  gui_transparency_(context->subVar("transparency"), 0),
  gui_active_isoval_selection_tab_(context->subVar("active-isoval-selection-tab"),
				   "1"),
  gui_active_tab_(context->subVar("active_tab"), "0"),
  gui_update_type_(context->subVar("update_type"), "Manual"),
  gui_color_r_(context->subVar("color-r"), 0.4),
  gui_color_g_(context->subVar("color-g"), 0.2),
  gui_color_b_(context->subVar("color-b"), 0.9),
  gui_color_a_(context->subVar("color-a"), 0.9),
  gui_num_threads_(context->subVar("num-threads"),0),
  default_color_changed_(false)
{
  marchingcubes_.set_progress_reporter(this);
}

void
ExtractIsosurface::execute()
{
  // Report things that do not make sense
  if( gui_build_field_.get() && (oport_connected("Surface") == false))
  {
    warning("Asking for a field to be extracted, but there is no output field port connection -- you may want to deselect the Build Output Field checkbox to increase performance.");
  }
  else if( !gui_build_field_.get() && oport_connected("Surface") )
  {
    warning("An output field port connection was found but no field is being extracted");
  }
  
  if( gui_build_geom_.get() && (oport_connected("Geometry") == false))
  {
    warning("Asking for geometry to be extracted, but there is no output geometry port connection -- you may want to deselect the Build Output Geometry checkbox to increase performance.");
  }
  else if( !gui_build_geom_.get() && oport_connected("Geometry"))
  {
    warning("An output geometry port connection was found but no geometry is being extracted");
  }
  
  FieldHandle field_input_handle;
  get_input_handle( "Field", field_input_handle, true );

  // Get the optional colormap for the geometry.
  ColorMapHandle colormap_input_handle;
  get_input_handle( "Optional Color Map", colormap_input_handle, false );

  // Inform module that execution started
  update_state(Executing);

  // Check to see if the input field has changed.
  if( inputs_changed_ )
  {

    std::pair<double, double> minmax;
    field_input_handle->vfield()->minmax(minmax.first, minmax.second);

    // Check to see if the gui min max are different than the field.
    if( gui_iso_value_min_.get() != minmax.first ||
        gui_iso_value_max_.get() != minmax.second )
    {
      gui_iso_value_min_.set( minmax.first );
      gui_iso_value_max_.set( minmax.second );

      std::ostringstream str;
      str << get_id() << " set_min_max ";
      TCLInterface::execute(str.str());
    }
  }

  std::vector<double> isovals;
  
  double qmin = gui_iso_value_min_.get();
  double qmax = gui_iso_value_max_.get();

  if (gui_active_isoval_selection_tab_.get() == "0") 
  { // slider / typed
    const double val = gui_iso_value_.get();
    const double valTyped = gui_iso_value_typed_.get();
    if (val != valTyped) 
    {
      std::ostringstream str;
      str << "Typed isovalue "<<valTyped<<
      " was out of range.  Using isovalue "<<val<<" instead";
      warning(str.str());
      gui_iso_value_typed_.set(val);
    }
    if ( qmin <= val && val <= qmax )
    {
      isovals.push_back(val);
    }
    else 
    {
      error("Typed isovalue out of range -- skipping isosurfacing.");
      return;
    }
  } 
  else if (gui_active_isoval_selection_tab_.get() == "1") 
  { // quantity
    int num = gui_iso_value_quantity_.get();

    if (num < 1) 
    {
      error("ExtractIsosurface quantity must be at least one -- skipping isosurfacing.");
      return;
    }

    std::string range = gui_iso_quantity_range_.get();

    if (range == "colormap") 
    {
      if (colormap_input_handle.get_rep() == 0) 
      {
        error("No color colormap for isovalue quantity");
        return;
      }
      qmin = colormap_input_handle->getMin();
      qmax = colormap_input_handle->getMax();
    } 
    else if (range == "manual") 
    {
      qmin = gui_iso_quantity_min_.get();
      qmax = gui_iso_quantity_max_.get();
    } // else we're using "field" and qmax and qmin were set above
    
    if (qmin >= qmax) 
    {
      error("Can't use quantity tab if the minimum and maximum are the same.");
      return;
    }

    std::string clusive = gui_iso_quantity_clusive_.get();

    std::ostringstream str;

    str << get_id() << " set-isoquant-list \"";

    if (clusive == "exclusive") 
    {
      // if the min - max range is 2 - 4, and the user requests 3 isovals,
      // the code below generates 2.333, 3.0, and 3.666 -- which is nice
      // since it produces evenly spaced slices in torroidal data.
	
      double di=(qmax - qmin)/(double)num;
      for (int i=0; i<num; i++) 
      {
        isovals.push_back(qmin + ((double)i+0.5)*di);
        str << " " << isovals[i];
      }
    } 
    else if (clusive == "inclusive") 
    {
      // if the min - max range is 2 - 4, and the user requests 3 isovals,
      // the code below generates 2.0, 3.0, and 4.0.

      double di=(qmax - qmin)/(double)(num-1.0);
      for (int i=0; i<num; i++) 
      {
        isovals.push_back(qmin + ((double)i*di));
        str << " " << isovals[i];
      }
    }

    str << "\"";
    TCLInterface::execute(str.str().c_str());

  } 
  else if (gui_active_isoval_selection_tab_.get() == "2") 
  { // list
    std::istringstream vlist(gui_iso_value_list_.get());
    double val;
    while(!vlist.eof()) 
    {
      vlist >> val;
      if (vlist.fail()) 
      {
        if (!vlist.eof()) 
        {
          vlist.clear();
          warning("List of Isovals was bad at character " +
            to_string((int)(vlist.tellg())) +
            "('" + ((char)(vlist.peek())) + "').");
        }
        break;
      }
      else if (!vlist.eof() && vlist.peek() == '%') 
      {
        vlist.get();
        val = qmin + (qmax - qmin) * val / 100.0;
      }
      isovals.push_back(val);
    }
  } 
  else if (gui_active_isoval_selection_tab_.get() == "3")
  { // matrix

    MatrixHandle matrix_input_handle;
    get_input_handle( "Optional Isovalues", matrix_input_handle, true );
    
    std::ostringstream str;
    str << get_id() << " set-isomatrix-list \"";

    for (size_type i=0; i < matrix_input_handle->nrows(); i++) 
    {
      for (size_type j=0; j < matrix_input_handle->ncols(); j++) 
      {
        isovals.push_back(matrix_input_handle->get(i, j));
        str << " " << isovals[i];
      }
    }

    str << "\"";
    TCLInterface::execute(str.str().c_str());
  } 
  else 
  {
    error("Bad active_isoval_selection_tab value");
    return;
  }

  // See if any of the isovalues have changed.
  if( isovals_.size() != isovals.size() ) 
  {
    isovals_.resize( isovals.size() );
    inputs_changed_ = true;
  }

  for( size_t i=0; i<isovals.size(); i++ )
  {
    if( isovals_[i] != isovals[i] ) 
    {
      isovals_[i] = isovals[i];
      inputs_changed_ = true;
    }
  }

  if( gui_build_field_.changed() ||
      gui_build_geom_.changed() ||
      gui_num_threads_.changed()) 
  {
    inputs_changed_ = true;
  }

  if( gui_build_geom_.get() &&
      ( default_color_changed_  == true ||
	gui_transparency_.changed() ||
	gui_color_r_.changed() ||
	gui_color_g_.changed() ||
	gui_color_b_.changed() ) )
  {
    inputs_changed_ = true;
  }

  // Decide if an interpolant will be computed for the output field.
  const bool build_node_interp = 
    gui_build_field_.get() && oport_connected("Node Mapping");

  const bool build_elem_interp =
    gui_build_field_.get() && oport_connected("Elem Mapping");

  FieldHandle field_output_handle;
  GeomHandle geometry_handle;
  MatrixHandle node_interpolant_handle; 
  MatrixHandle elem_interpolant_handle; 

  if( (gui_build_field_.get() && !oport_cached("Surface")) ||
      (gui_build_geom_.get()  && !geometry_handle.get_rep()) ||
      (gui_build_geom_.get()  && !oport_cached("Geometry")) ||
      (build_node_interp      && !oport_cached("Node Mapping")) ||
      (build_elem_interp      && !oport_cached("Elem Mapping")) ||
      inputs_changed_  ) 
  {
      // Inform module that execution started
      update_state(Executing);
      if (colormap_input_handle.get_rep()) {
        marchingcubes_.set_colormap("colormap", colormap_input_handle.get_rep());
      }
      else {
        marchingcubes_.set_colormap("colormap", 0);
      }
      
      // algorithm chooses between color and colormap
      marchingcubes_.set_color("color",
        Color(gui_color_r_.get(), gui_color_g_.get(), gui_color_b_.get()));

      default_color_changed_ = false;

      marchingcubes_.set_colormap("colormap",colormap_input_handle.get_rep());
      marchingcubes_.set_bool("build_geometry",gui_build_geom_.get());
      marchingcubes_.set_bool("build_field",gui_build_field_.get());
      marchingcubes_.set_bool("build_node_interpolant",build_node_interp);
      marchingcubes_.set_bool("build_elem_interpolant",build_elem_interp);
      marchingcubes_.set_int("num_threads",gui_num_threads_.get());

    if(!( marchingcubes_.run(field_input_handle,isovals_,
                             geometry_handle,field_output_handle,
                             node_interpolant_handle,
                             elem_interpolant_handle)))
    {
      error("Marching Cubes algorithm failed");
      return;
    }

    if (field_output_handle.get_rep())
    {
      std::string fldname;
      if (field_input_handle->get_property("name",fldname)) 
      {
        field_output_handle->set_property("name",fldname, false);
      } 
      else 
      {
        field_output_handle->set_property("name", std::string("ExtractIsosurface"), false);
      }    

      // Send the isosurface field downstream
      send_output_handle( "Surface", field_output_handle );
    }
    
    // Get the output node interpolant handle.
    if (build_node_interp) 
    {
      send_output_handle( "Node Mapping", node_interpolant_handle );
    }

    // Get the output cell interpolant handle.
    if (build_elem_interp) 
    {
      send_output_handle( "Elem Mapping", elem_interpolant_handle );
    }

    // Merged the geometry results.
    if( gui_build_geom_.get() && isovals.size() ) 
    {
      std::string fldname;
      if (!field_input_handle->get_property("name", fldname))
        fldname = std::string("ExtractIsosurface");

      send_output_handle( "Geometry", geometry_handle, fldname );
    }
  }
}


void ExtractIsosurface::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2) 
  {
    args.error("ExtractIsosurface needs a minor command");
    return;
  }

  if (args[1] == "default_color_change") 
  {
    default_color_changed_ = true;
  } 
  else 
  {
    Module::tcl_command(args, userdata);
  }
}

} // End namespace SCIRun
