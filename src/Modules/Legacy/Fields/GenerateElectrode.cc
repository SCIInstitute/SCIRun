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


///@brief This module makes a mesh that looks like a wire

#include <Modules/Legacy/Fields/GenerateElectrode.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Logging/Log.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Graphics/Widgets/WidgetFactory.h>

using namespace SCIRun;
using namespace Core;
using namespace Logging;
using namespace Datatypes;
using namespace Algorithms;
using namespace Geometry;
using namespace Graphics;
using namespace Modules::Fields;
using namespace Dataflow::Networks;
using namespace Graphics::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;

MODULE_INFO_DEF(GenerateElectrode, NewField, SCIRun)

ALGORITHM_PARAMETER_DEF(Fields, ElectrodeLength);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeThickness);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeWidth);
ALGORITHM_PARAMETER_DEF(Fields, NumberOfControlPoints);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeType);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeResolution);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeProjection);
ALGORITHM_PARAMETER_DEF(Fields, MoveAll);
ALGORITHM_PARAMETER_DEF(Fields, UseFieldNodes);

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Fields
      {
        class GenerateElectrodeImpl
        {
        public:
          void get_points(std::vector<Point>& points);
          void get_centers(std::vector<Point>& p, std::vector<Point>& pp, double length, int resolution);
          FieldHandle Make_Mesh_Wire(std::vector<Point>& points, double thickness, int resolution);

          std::vector<Point> Previous_points_;
        };
      }
    }
  }
}

#if 0
	class GenerateElectrode : public Module
		{
		public:
			GenerateElectrode(GuiContext* ctx);
			~GenerateElectrode(){};
			virtual void execute();
			virtual void widget_moved(bool, BaseWidget*);
			virtual void tcl_command(GuiArgs& args, void* userdata);

			virtual void post_read(); // get the widget state...
			virtual void presave();

			void add_point(std::vector<Point>& p);
			bool remove_point();

			void create_widgets(std::vector<Point>& points);

			void create_widgets(std::vector<Point>& points,Vector& direction);

			void Make_Mesh_Planar(std::vector<Point>& points, FieldHandle& ofield, Vector& direction);


		private:

			CrowdMonitor										widget_lock_;
			GeomHandle											widget_switch_;
			GeometryOPortHandle							geom_oport_;

			std::vector<PointWidget*>				widget_;
			ArrowWidget*										arrow_widget_;
			GuiVectorHandle									widget_direction_;

			std::vector<GuiPointHandle>			widget_point_;
			GuiDouble												gui_probe_scale_;

			GuiDouble												gui_color_r_;
			GuiDouble												gui_color_g_;
			GuiDouble												gui_color_b_;

			GuiInt													gui_widget_points_;

			GuiDouble												gui_length_;
			GuiDouble												gui_width_;
			GuiDouble												gui_thick_;

			GuiString												gui_moveto_;
			GuiString												gui_type_;
			GuiString												gui_project_;

			GuiInt													gui_use_field_;
			GuiInt													gui_move_all_;
			GuiInt													gui_wire_res_;

			bool														color_changed_;
			bool														move_all_;




		};
#endif

GenerateElectrode::GenerateElectrode() : GeometryGeneratingModule(staticInfo_),
  impl_(new GenerateElectrodeImpl)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(ElectrodeWidget);
  INITIALIZE_PORT(ControlPoints);
}

void GenerateElectrode::setStateDefaults()
{
  auto state = get_state();

  state->setValue(Parameters::ElectrodeLength, 0.1);
  state->setValue(Parameters::ElectrodeThickness, 0.003);
  state->setValue(Parameters::NumberOfControlPoints, 5);
  state->setValue(Parameters::ElectrodeResolution, 10);
  state->setValue(Parameters::ElectrodeType, std::string("wire"));
}

void GenerateElectrode::execute()
{
  FieldHandle ofield, pfield;

  //TODO: enable optional input logic
  auto source = getRequiredInput(InputField);

//  FieldInformation fis(source);
//  std::vector<Point> orig_points;
//  Vector direction;
//  Vector defdir = Vector(-10, 10, 10);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  const std::string &moveto = gui_moveto_.get();
  int use_field = gui_use_field_.get();
#endif

//  auto state = get_state();
//  auto electrode_type = state->getValue(Parameters::ElectrodeType).toString();

//  if (source
//#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
//    && (use_field == 1) && (moveto == "default" || widget_.size() == 0 || inputs_changed_)
//#endif
//    )
//  {
//    VMesh* smesh = source->vmesh();
//
//    smesh->synchronize(Mesh::ELEM_LOCATE_E);
//
//    VMesh::Node::size_type num_nodes = smesh->num_nodes();
//    if (num_nodes > 50)
//    {
//      error("Why would you want to use that many nodes to make an electrode?  Do you want to crash you system?  That's way to many.");
//      return;
//    }
//
//    VMesh::Node::array_type a;
//    orig_points.resize(num_nodes);
//
//    for (VMesh::Node::index_type idx = 0; idx < num_nodes; idx++)
//    {
//      Point ap;
//      smesh->get_center(ap, idx);
//
//      orig_points[idx] = ap;
//      direction = defdir;
//    }
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    gui_moveto_.set("");
#endif
//  }
//#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
//  else if ((!input_field_p || use_field == 0) && (moveto == "default" || widget_.size() == 0))
//  {
//    double l, lx;
//    l = gui_length_.get();
//
//    lx = l * .5774;
//
//    orig_points.resize(5);
//
//    orig_points[0] = (Point(0, 0, 0));
//    orig_points[1] = (Point(lx*.25, lx*.25, lx*.25));
//    orig_points[2] = (Point(lx*.5, lx*.5, lx*.5));
//    orig_points[3] = (Point(lx*.75, lx*.75, lx*.75));
//    orig_points[4] = (Point(lx, lx, lx));
//
//    direction = defdir;
//    gui_moveto_.set("");
//  }
//  else if (moveto == "add_point")
//  {
//    add_point(orig_points);
//    if (electrode_type == "planar")
//    {
//      direction = arrow_widget_->GetDirection();
//    }
//    else
//    {
//      direction = defdir;
//    }
//    gui_moveto_.set("");
//  }
//  else if (moveto == "remove_point")
//  {
//    remove_point();
//    gui_moveto_.set("");
//    return;
//  }
//  else
//  {
//    size_t n = widget_.size(), s = 0;
//    orig_points.resize(n);
//    direction = defdir;
//
//    if (arrow_widget_)
//    {
//      n = n + 1;
//      s = 1;
//      orig_points.resize(n);
//      direction = arrow_widget_->GetDirection();
//      orig_points[0] = arrow_widget_->GetPosition();
//    }
//
//    for (size_t k = s; k < n; k++)
//    {
//      orig_points[k] = widget_[k - s]->GetPosition();
//    }
//  }
//#endif


//#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
//  gui_widget_points_.set(orig_points.size());
//
//  if (electrode_type == "wire")
//    arrow_widget_ = 0;
//#endif
//
//  if (impl_->Previous_points_.size() < 3)
//  {
//    impl_->Previous_points_ = orig_points;
//  }
//
//  size_type size = orig_points.size();
//
//  Vector move_dist;
//  std::vector<Point> temp_points;
//
//#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
//  if (move_all_)
//  {
//    for (size_t k = 0; k < size; k++)
//    {
//      if (orig_points[k] != Previous_points_[k])
//      {
//        move_dist = orig_points[k] - Previous_points_[k];
//        move_idx = k;
//      }
//    }
//
//    for (size_t k = 0; k < size; k++)
//    {
//      if (k == move_idx) temp_points.push_back(orig_points[k]);
//      else temp_points.push_back(orig_points[k] + move_dist);
//    }
//    orig_points = temp_points;
//    move_all_ = false;
//  }
//#endif
//
//  std::vector<Point> final_points;
//  std::vector<Point> points(size);
//
//#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER // Tark
//  if (electrode_type == "wire")
//    create_widgets(orig_points);
//  if (electrode_type == "planar")
//    create_widgets(orig_points, direction);
//#endif
//
//  impl_->Previous_points_ = orig_points;
//
//  FieldInformation pi("PointCloudMesh", 0, "double");
//  MeshHandle pmesh = CreateMesh(pi);
//
//  for (VMesh::Node::index_type idx = 0; idx < orig_points.size(); idx++) pmesh->vmesh()->add_point(orig_points[idx]);
//
//  //TODO: copy this here since widgets don't exist yet
//  points = orig_points;
//
//  pi.make_double();
//  pfield = CreateField(pi, pmesh);

  sendOutputFromAlgo;

//  impl_->get_centers(points, final_points,
//    state->getValue(Parameters::ElectrodeLength).toDouble(),
//    state->getValue(Parameters::ElectrodeResolution).toInt());
//
//  if (electrode_type == "wire")
//    sendOutput(OutputField, impl_->Make_Mesh_Wire(final_points,
//      state->getValue(Parameters::ElectrodeThickness).toDouble(),
//      state->getValue(Parameters::ElectrodeResolution).toInt()));
//
//#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
//  if (electrode_type == "planar")
//    Make_Mesh_Planar(final_points, ofield, direction);
//#endif
    
}





#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

	void
	GenerateElectrode::widget_moved(bool release, BaseWidget* widget_)
	{
		if (release) want_to_execute();
		if (gui_move_all_.get()) move_all_=true;
	}


	void
	GenerateElectrode::create_widgets(std::vector<Point>& points)
	{
		GeomGroup *group = new GeomGroup;
		widget_switch_ = new GeomSwitch(group);

		geom_oport_->delAll();
		widget_.clear();

		double scale;
		scale=gui_probe_scale_.get()*gui_thick_.get()*.5;


		for (size_t k = 0; k < points.size(); k++)
		{
			PointWidget *widget =  new PointWidget(this, &widget_lock_,gui_probe_scale_.get());
			widget_.push_back(widget);

			widget->Connect(geom_oport_.get_rep());
			widget->SetCurrentMode(0);
			widget->SetScale(scale);
			widget->SetPosition(points[k]);
			widget->SetColor(Color(gui_color_r_.get(),gui_color_g_.get(),gui_color_b_.get()));

			group->add(widget->GetWidget().get_rep());
		}

		geom_oport_->addObj(widget_switch_,"Wire Electrode", &widget_lock_);
	}

	void
	GenerateElectrode::create_widgets(std::vector<Point>& points,Vector& direction)
	{
		GeomGroup *group = new GeomGroup;
		widget_switch_ = new GeomSwitch(group);

		geom_oport_->delAll();
		widget_.clear();


		double scale;
		scale=gui_probe_scale_.get()*gui_thick_.get()*.5;

		ArrowWidget *awidget =new ArrowWidget(this, &widget_lock_,scale);
		awidget->SetDirection(direction);
		awidget->SetScale(scale);
		awidget->SetLength(scale*4);
		awidget->SetPosition(points[0]);

		arrow_widget_=awidget;

		group->add(awidget->GetWidget().get_rep());

		for (size_t k = 1; k < points.size(); k++)
		{
			PointWidget *widget =  new PointWidget(this, &widget_lock_,gui_probe_scale_.get());


			widget->Connect(geom_oport_.get_rep());
			widget->SetCurrentMode(0);
			widget->SetScale(scale);
			widget->SetPosition(points[k]);
			widget->SetColor(Color(gui_color_r_.get(),gui_color_g_.get(),gui_color_b_.get()));

			widget_.push_back(widget);

			group->add(widget->GetWidget().get_rep());
		}

		geom_oport_->addObj(widget_switch_,"Wire Electrode", &widget_lock_);
	}
#endif

void GenerateElectrodeImpl::get_points(std::vector<Point>& points)
{
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
	size_t s=0,n=widget_.size();
	points.resize(n);


	if(gui_type_.get()=="planar")
	{
		n+=1;
		s=1;
		points.resize(n);
		points[0]=arrow_widget_->GetPosition();
	}


	for (size_t k = s; k < n; k++)
    points[k] = widget_[k-s]->GetPosition();
#endif
  //TODO: defaulting widget positions to hard-coded values.
  // Use input field points instead.
  points = { {0,0,0}, {1,0,0}, {2,0,0}, {3,0,0}, {4,0,0} };
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
	void
	GenerateElectrode::tcl_command(GuiArgs& args, void* userdata)
	{
		if(args.count() < 2)
		{
			args.error("ShowString needs a minor command");
			return;
		}

		if (args[1] == "color_change")
		{
			color_changed_ = true;
		}
		else
		{
			Module::tcl_command(args, userdata);
		}
	}

	void
	GenerateElectrode::presave()
	{
		//cout<<"Started presave()"<<endl;

		//cout<<"Electrode Type:  "<<gui_type_.get()<<endl;

		size_t has_arrow = 0;
		size_t num_points = widget_.size();

		widget_point_.clear();


		if(gui_type_.get()=="planar")
		{
			//cout<<"Started if statement"<<endl;

			if (arrow_widget_)
			{
				has_arrow = 1;

				widget_direction_=new GuiVector(get_ctx()->subVar("widget-direction-"+to_string(0)));
				widget_direction_->set(arrow_widget_->GetDirection());

				widget_point_.push_back(new GuiPoint(get_ctx()->subVar("widget-point-"+to_string(0))));
				widget_point_[0]->set(arrow_widget_->GetPosition());
			}
			//cout<<"ending if statement"<<endl;

		}

		for (size_t k = has_arrow; k< num_points+has_arrow; k++)
		{
			widget_point_.push_back(new GuiPoint(get_ctx()->subVar("widget-point-"+to_string(k))));
			widget_point_[k]->set(widget_[k-has_arrow]->GetPosition());
		}

		gui_widget_points_.set(num_points+has_arrow);

		//cout<<"finished presave()"<<endl;

	}

	void
	GenerateElectrode::post_read()
	{
		size_t has_arrow = 0;
		size_t num_points = gui_widget_points_.get();
		Vector direction;

		widget_point_.clear();
		std::vector<Point> points;

		if(gui_type_.get()=="planar")
		{
			has_arrow = 1;
			widget_direction_= new GuiVector(get_ctx()->subVar("widget-direction-"+to_string(0)));
			widget_direction_->reset();
			direction=widget_direction_->get();

			widget_point_.push_back(new GuiPoint(get_ctx()->subVar("widget-point-"+to_string(0))));
			widget_point_[0]->reset();
			points.push_back(widget_point_[0]->get());
		}


		for (size_t k = has_arrow; k< num_points; k++)
		{
			widget_point_.push_back(new GuiPoint(get_ctx()->subVar("widget-point-"+to_string(k))));
			widget_point_[k]->reset();
			points.push_back(widget_point_[k]->get());
		}

		if(gui_type_.get()=="wire") create_widgets(points);
		if(gui_type_.get()=="planar") create_widgets(points,direction);

	}

	void
	GenerateElectrode::add_point(std::vector<Point>& p)
	{


		size_t size=widget_.size(), s=0;
		std::vector<Point> points(size);

		if(gui_type_.get()=="planar")
		{
			size+=1;
			s=1;
			points.resize(size);
			points[0]=arrow_widget_->GetPosition();
		}

		for (size_t k = s; k < size; k++) points[k] = widget_[k-s]->GetPosition();

		p.resize(size+1);

		for (size_t k = 0; k < size-1; k++) p[k]=points[k];

		p[size]=points[size-1];
		p[size-1]=Point(points[size-2]+(points[size-1]-points[size-2])*.5);



	}


	bool
	GenerateElectrode::remove_point()
	{
		//cout<<"---removing a widget with remove button"<<endl;

		size_t n;

		if(gui_type_.get()=="wire") n=3;
		if(gui_type_.get()=="planar") n=2;

		if (widget_.size() > n)
		{
			widget_.pop_back();
		}
		else
		{
			error("Must have at least 3 points.");
		}

		want_to_execute();

		return (true);
	}
#endif
