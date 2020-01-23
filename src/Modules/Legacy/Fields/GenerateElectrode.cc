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
ALGORITHM_PARAMETER_DEF(Fields, NumberOfControlPoints);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeType);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeResolution);

//TODO: move to algorithm layer
namespace impl
{
  // equivalent to the interp1 command in matlab.  uses the parameters p and t to perform a cubic spline interpolation pp in one direction.

  bool CalculateSpline(std::vector<double>& t, std::vector<double>& x, std::vector<double>& tt, std::vector<double>& xx)
  {
    // need to have at least 3 nodes
    if (t.size() < 3) return (false);
    if (x.size() != t.size()) return (false);

    size_t size = x.size();
    std::vector<double> z(size), h(size - 1), b(size - 1), v(size - 1), u(size - 1);

    for (size_t k = 0; k < size - 1; k++)
    {
      h[k] = (t[k + 1] - t[k]);
      b[k] = (6 * (x[k + 1] - x[k]) / h[k]);
    }

    u[1] = 2 * (h[0] + h[1]);
    v[1] = b[1] - b[0];

    for (size_t k = 2; k < size - 1; k++)
    {
      u[k] = 2 * (h[k] + h[k - 1]) - (h[k - 1] * h[k - 1]) / u[k - 1];
      v[k] = b[k] - b[k - 1] - h[k - 1] * v[k - 1] / u[k - 1];
    }

    z[size - 1] = 0;

    for (size_t k = size - 2; k > 0; k--)
    {
      z[k] = (v[k] - h[k] * z[k + 1]) / u[k];
    }

    z[0] = 0;

    size_t segment = 0;

    xx.resize(tt.size());
    for (size_t k = 0; k < tt.size(); k++)
    {
      while (segment < (size - 2) && t[segment + 1] < tt[k])
      {
        segment++;
      }

      double w0, w1, w2, w3, a, b, c, d;

      w3 = (t[segment + 1] - tt[k]);
      w0 = w3 * w3*w3;
      w2 = (tt[k] - t[segment]);
      w1 = w2 * w2*w2;

      a = z[segment] / (6 * h[segment]);
      b = z[segment + 1] / (6 * h[segment]);
      c = (x[segment + 1] / h[segment] - (z[segment + 1] * h[segment]) / 6);
      d = (x[segment] / h[segment] - z[segment] * h[segment] / 6);

      xx[k] = a * w0 + b * w1 + c * w2 + d * w3;
    }

    return (true);
  }

  // this is a spline function.  pp is the final points that are in between the original points p.
  // t and tt are the original and final desired spacing, respectively.

  bool CalculateSpline(std::vector<double>& t, std::vector<Point>& p, std::vector<double>& tt, std::vector<Point>& pp)
  {
    // need to have at least 3 nodes
    if (t.size() < 3) return (false);
    if (p.size() != t.size()) return (false);

    size_t size = p.size();

    std::vector<double> x(size), y(size), z(size);
    std::vector<double> xx, yy, zz;

    for (size_t k = 0; k < p.size(); k++)
    {
      x[k] = p[k].x();
      y[k] = p[k].y();
      z[k] = p[k].z();
    }

    CalculateSpline(t, x, tt, xx);
    CalculateSpline(t, y, tt, yy);
    CalculateSpline(t, z, tt, zz);

    for (size_t k = 0; k < tt.size(); k++)
      pp.emplace_back(xx[k], yy[k], zz[k]);

    return (true);
  }
}

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
		//widget_lock_("GenerateElectrode widget lock"),
		//arrow_widget_(0),
		//gui_probe_scale_(get_ctx()->subVar("probe_scale"), 3.0),
		//gui_color_r_(get_ctx()->subVar("color-r"), 1.0),
		//gui_color_g_(get_ctx()->subVar("color-g"), 1.0),
		//gui_color_b_(get_ctx()->subVar("color-b"), 1.0),
		//gui_widget_points_(get_ctx()->subVar("num_points"),5),
		//gui_length_(get_ctx()->subVar("length"),.1),
		//gui_width_(get_ctx()->subVar("width"),.02),
		//gui_thick_(get_ctx()->subVar("thick"),.003),
		//gui_moveto_(get_ctx()->subVar("moveto"), ""),
		//gui_project_(get_ctx()->subVar("project"),"midway"),
		//gui_use_field_(get_ctx()->subVar("use-field"),1),
		//gui_move_all_(get_ctx()->subVar("move-all"),0),
		//gui_wire_res_(get_ctx()->subVar("wire_res"),10)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(ParameterMatrixIn);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(ElectrodeWidget);
  INITIALIZE_PORT(ControlPoints);
  INITIALIZE_PORT(ParameterMatrixOut);
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
  MatrixHandle source_matrix;

  //TODO: enable optional input logic
  auto source = getRequiredInput(InputField);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  const bool input_matrix_p = get_input_handle("Parameter Matrix", source_matrix, false);
#endif

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  size_type num_para = 0;
  size_type num_col = 0;

  if (input_matrix_p)
  {
    num_para = source_matrix->nrows();
    num_col = source_matrix->ncols();
  }

  if (input_matrix_p && num_para == 5 && num_col == 1)
  {
    double* sm = source_matrix->get_data_pointer();

    double temp = sm[0];
    gui_length_.set(temp);
    temp = sm[1];
    gui_width_.set(temp);
    temp = sm[2];
    gui_thick_.set(temp);
    temp = sm[3];
    gui_wire_res_.set(static_cast<int> (temp));
    temp = sm[4];

    if (temp == 1) gui_type_.set("planar");
    else if (temp == 0) gui_type_.set("wire");
    else
    {
      error("Last value in the input matrix needs to be 1 or 0");
      return;
    }
  }

  if (input_matrix_p && (num_para != 5 || num_col != 1))
  {
    error("Parameter matrix needs to be right size (1x5). This input is optional, but remember: Length, Width, Thickness, Resolution, Type (1=planar, 0=wire)");
    return;
  }
#endif

  FieldInformation fis(source);
  std::vector<Point> orig_points;
  Vector direction;
  Vector defdir = Vector(-10, 10, 10);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  const std::string &moveto = gui_moveto_.get();
  int use_field = gui_use_field_.get();
#endif

  auto state = get_state();
  auto electrode_type = state->getValue(Parameters::ElectrodeType).toString();

  if (source
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    && (use_field == 1) && (moveto == "default" || widget_.size() == 0 || inputs_changed_)
#endif
    )
  {
    VMesh* smesh = source->vmesh();

    smesh->synchronize(Mesh::ELEM_LOCATE_E);

    VMesh::Node::size_type num_nodes = smesh->num_nodes();
    if (num_nodes > 50)
    {
      error("Why would you want to use that many nodes to make an electrode?  Do you want to crash you system?  That's way to many.");
      return;
    }

    VMesh::Node::array_type a;
    orig_points.resize(num_nodes);

    for (VMesh::Node::index_type idx = 0; idx < num_nodes; idx++)
    {
      Point ap;
      smesh->get_center(ap, idx);

      orig_points[idx] = ap;
      direction = defdir;
    }
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    gui_moveto_.set("");
#endif
  }
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  else if ((!input_field_p || use_field == 0) && (moveto == "default" || widget_.size() == 0))
  {
    double l, lx;
    l = gui_length_.get();

    lx = l * .5774;

    orig_points.resize(5);

    orig_points[0] = (Point(0, 0, 0));
    orig_points[1] = (Point(lx*.25, lx*.25, lx*.25));
    orig_points[2] = (Point(lx*.5, lx*.5, lx*.5));
    orig_points[3] = (Point(lx*.75, lx*.75, lx*.75));
    orig_points[4] = (Point(lx, lx, lx));

    direction = defdir;
    gui_moveto_.set("");
  }
  else if (moveto == "add_point")
  {
    add_point(orig_points);
    if (electrode_type == "planar")
    {
      direction = arrow_widget_->GetDirection();
    }
    else
    {
      direction = defdir;
    }
    gui_moveto_.set("");
  }
  else if (moveto == "remove_point")
  {
    remove_point();
    gui_moveto_.set("");
    return;
  }
  else
  {
    size_t n = widget_.size(), s = 0;
    orig_points.resize(n);
    direction = defdir;

    if (arrow_widget_)
    {
      n = n + 1;
      s = 1;
      orig_points.resize(n);
      direction = arrow_widget_->GetDirection();
      orig_points[0] = arrow_widget_->GetPosition();
    }

    for (size_t k = s; k < n; k++)
    {
      orig_points[k] = widget_[k - s]->GetPosition();
    }
  }
#endif


#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  gui_widget_points_.set(orig_points.size());

  if (electrode_type == "wire")
    arrow_widget_ = 0;
#endif

  if (impl_->Previous_points_.size() < 3)
  {
    impl_->Previous_points_ = orig_points;
  }

  size_type size = orig_points.size();

  Vector move_dist;
  std::vector<Point> temp_points;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (move_all_)
  {
    for (size_t k = 0; k < size; k++)
    {
      if (orig_points[k] != Previous_points_[k])
      {
        move_dist = orig_points[k] - Previous_points_[k];
        move_idx = k;
      }
    }

    for (size_t k = 0; k < size; k++)
    {
      if (k == move_idx) temp_points.push_back(orig_points[k]);
      else temp_points.push_back(orig_points[k] + move_dist);
    }
    orig_points = temp_points;
    move_all_ = false;
  }
#endif

  std::vector<Point> final_points;
  std::vector<Point> points(size);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER // Tark
  if (electrode_type == "wire")
    create_widgets(orig_points);
  if (electrode_type == "planar")
    create_widgets(orig_points, direction);
#endif

  impl_->Previous_points_ = orig_points;

  FieldInformation pi("PointCloudMesh", 0, "double");
  MeshHandle pmesh = CreateMesh(pi);

  for (VMesh::Node::index_type idx = 0; idx < orig_points.size(); idx++) pmesh->vmesh()->add_point(orig_points[idx]);

  //TODO: copy this here since widgets don't exist yet
  points = orig_points;

  pi.make_double();
  pfield = CreateField(pi, pmesh);

  sendOutput(ControlPoints, pfield);

  impl_->get_centers(points, final_points,
    state->getValue(Parameters::ElectrodeLength).toDouble(),
    state->getValue(Parameters::ElectrodeResolution).toInt());

  if (electrode_type == "wire")
    sendOutput(OutputField, impl_->Make_Mesh_Wire(final_points,
      state->getValue(Parameters::ElectrodeThickness).toDouble(),
      state->getValue(Parameters::ElectrodeResolution).toInt()));

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (electrode_type == "planar")
    Make_Mesh_Planar(final_points, ofield, direction);
#endif

  DenseMatrixHandle parameters(new DenseMatrix(5, 1));
  (*parameters) << state->getValue(Parameters::ElectrodeLength).toDouble(),
  0, // TODO wire has no width--gui_width_.get();
  state->getValue(Parameters::ElectrodeThickness).toDouble(),
  state->getValue(Parameters::ElectrodeResolution).toInt(),
  electrode_type == "wire" ? 0 : 1;

  sendOutput(ParameterMatrixOut, parameters);
}

	FieldHandle GenerateElectrodeImpl::Make_Mesh_Wire(std::vector<Point>& final_points, double thickness, int resolution)
	{
		FieldInformation fi("TetVolMesh",0,"double");
		MeshHandle mesh = CreateMesh(fi);
		VMesh::Node::array_type nodes;

		double Pi=3.14159;

		double radius = thickness *.5;

		size_t DN = resolution;

		Vector Vold1, Vold2;
		Vector V1, V2, V, Vx, Vy;

		Vold1[0]=1;
		Vold1[1]=0;
		Vold1[2]=0;

		Vold2[0]=0;
		Vold2[1]=1;
		Vold2[2]=0;

		size_t N=final_points.size();

		std::vector<Point> p=final_points;
		std::vector<double> phi(DN);
		std::vector<Point> fin_nodes;

		for (size_t q=0;q<DN;q++)
		{
			phi[q]=q*(2*Pi/(static_cast<double> (DN)));
		}

		for (size_t k=0;k<N;k++)
		{
			if (k==N-1)
			{
				V1=p[k]-p[k-1];
				V2=V1;
			}
			else if (k==0)
			{
				V2=p[k+1]-p[k];
				V1=V2;
			}
			else
			{
				V1=p[k]-p[k-1];
				V2=p[k+1]-p[k];
			}


			if (sqrt(V1[0]*V1[0] + V1[1]*V1[1] + V1[2]*V1[2])>0)
			{
				V1.normalize();
			}
			else
			{
				V1[0]=1; V1[1]=0; V1[2]=0;
			}

			if (sqrt(V2[0]*V2[0] + V2[1]*V2[1] + V2[2]*V2[2])>0)
			{
				V2.normalize();
			}
			else
			{
				V2[0]=0; V2[1]=1; V2[2]=0;
			}


			V=(V1+V2)*.5;
			V.normalize();

			if (Dot(V,Vold1)<.9)
			{
				Vx=Cross(V,Vold1);
				Vy=Cross(V,Vx);
			}
			else
			{
				Vx=Cross(V,Vold2);
				Vy=Cross(V,Vx);
			}

			Vx.normalize();
			Vy.normalize();

			Vold1=-Vy;
			Vold2=Vx;

			fin_nodes.push_back(p[k]);

			for (size_t q=0;q<DN;q++)
			{
				fin_nodes.push_back(Point(p[k]+Vx*radius*cos(phi[q])+Vy*radius*sin(phi[q])));
			}
		}

		for (VMesh::Node::index_type idx=0;idx<fin_nodes.size();idx++)
		{
			mesh->vmesh()->add_point(fin_nodes[idx]);
			mesh->vmesh()->get_nodes(nodes,idx);
		}

		VMesh::Node::index_type SE=0, EE=0, SE1, SE2, EE1, EE2;
		std::vector<size_t>  L(DN+1);

		for (size_t k=0;k<DN;k++)
		{
			L[k]=k+1;
		}
		L[DN]=1;

		for (VMesh::Node::index_type idx=0;idx<final_points.size()-1;idx++)
		{
			SE=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+(static_cast<int> (DN))+1);

			for (VMesh::Node::index_type k=0;k<DN;k++)
			{
				SE1=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+(static_cast<int> (L[k])));
				SE2=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+(static_cast<int> (L[k+1])));
				EE1=static_cast<VMesh::Node::index_type> (static_cast<int> (EE)+(static_cast<int> (L[k])));
				EE2=static_cast<VMesh::Node::index_type> (static_cast<int> (EE)+(static_cast<int> (L[k+1])));

				VMesh::Node::array_type elem_nodes(4);

				elem_nodes[0]=EE;
				elem_nodes[1]=EE1;
				elem_nodes[2]=EE2;
				elem_nodes[3]=SE;

				mesh->vmesh()->add_elem(elem_nodes);

				elem_nodes[0]=EE1;
				elem_nodes[1]=EE2;
				elem_nodes[2]=SE2;
				elem_nodes[3]=SE;

				mesh->vmesh()->add_elem(elem_nodes);

				elem_nodes[0]=EE1;
				elem_nodes[1]=SE1;
				elem_nodes[2]=SE2;
				elem_nodes[3]=SE;

				mesh->vmesh()->add_elem(elem_nodes);
			}

			EE=static_cast<VMesh::Node::index_type> (static_cast<int> (EE)+(static_cast<int> (DN))+1);
		}

		fi.make_double();
		return CreateField(fi,mesh);
	}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

	void
	GenerateElectrode::Make_Mesh_Planar(std::vector<Point>& final_points, FieldHandle& ofield, Vector& direction)
	{
		//-------make planar mesh---------

		FieldInformation fi("TetVolMesh",0,"double");
		MeshHandle mesh = CreateMesh(fi);
		VMesh::Node::array_type nodes;

		bool vect_strangeness=false;
		bool res_strangeness=false;

		const std::string &proj=gui_project_.get();

		double aa, bb;

		if (proj=="positive")
		{
			aa=0;
			bb=1;
		}
		else if (proj=="midway")
		{
			aa=.5;
			bb=1;
		}
		else if (proj=="negative")
		{
			aa=0;
			bb=-1;
		}

		//cout <<"proj= "<<proj<<".  aa = "<<aa<<". bb "<<bb<<endl;


		Vector V1, V2, V, Vx, Vy, Vxold;

		double width=gui_width_.get()/2;
		double thick=gui_thick_.get();

		size_t N=final_points.size();

		std::vector<Point> fin_nodes;

		Vector direc=arrow_widget_->GetDirection();

		direction.normalize();

		std::vector<Point> p=final_points;

		Point srp_old, srn_old, pr_old;

		Vector temp1, temp2;
		double temp1_mag, temp2_mag;


		for (size_t k=0;k<N;k++)
		{

			if (k==N-1)
			{
				V1=p[k]-p[k-1];
				V2=V1;
			}

			else if (k==0)
			{
				V2=p[k+1]-p[k];
				V1=V2;
			}
			else
			{
				V1=p[k]-p[k-1];
				V2=p[k+1]-p[k];
			}

			if (sqrt(V1[0]*V1[0] + V1[1]*V1[1] + V1[2]*V1[2])>0)
			{
				V1.normalize();
			}
			else
			{
				V1[0]=1; V1[1]=0; V1[2]=0;
			}

			if (sqrt(V2[0]*V2[0] + V2[1]*V2[1] + V2[2]*V2[2])>0)
			{
				V2.normalize();
			}
			else
			{
				V2[0]=0; V2[1]=1; V2[2]=0;
			}

			V=(V1+V2)*.5;
			V.normalize();

			if (Dot(V1,direc)>.8)
			{
				vect_strangeness=true;
				//std::cout <<"V1 . direction = "<<Dot(V1,direc)<<std::endl;
			}

			Vx=Cross(V1,direc);
			if (Dot(Vx,Vxold)<.3  && k>0)
			{
				vect_strangeness=true;
				//std::cout <<"newx . oldx = "<<Dot(Vx,Vxold)<<std::endl;
				Vx=Vxold;
			}
			Vy=Cross(V1,Vx);





			/*
			 }
			 else
			 {
			 Vx=Cross(V2,direc);
			 Vy=Cross(V2,Vy);
			 }
			 */

			Vx.normalize();
			Vy.normalize();

			Point pr=Point(p[k]+Vy*thick*aa);

			fin_nodes.push_back(pr);
			fin_nodes.push_back(Point(pr-Vy*thick*bb));
			Point srp=Point(pr+Vx*width);
			fin_nodes.push_back(srp);
			fin_nodes.push_back(Point(pr+Vx*width-Vy*thick*bb));
			Point srn=Point(pr-Vx*width);
			fin_nodes.push_back(srn);
			fin_nodes.push_back(Point(pr-Vx*width-Vy*thick*bb));

			Vxold=Vx;

			if (k>0)
			{
				temp1=srp_old-pr;
				temp2=srn_old-pr;
				temp1_mag=sqrt(temp1[0]*temp1[0]+temp1[1]*temp1[1]+temp1[2]*temp1[2]);
				temp2_mag=sqrt(temp2[0]*temp2[0]+temp2[1]*temp2[1]+temp2[2]*temp2[2]);


				if (temp1_mag<width)
				{
					res_strangeness=true;
				}
			}

			srp_old=srp;
			srn_old=srn;
			pr_old=pr;
		}

		if (vect_strangeness)
		{
			warning("Vector is close to parrallel to part of the spline.  Consider adjusting");
		}

		if (res_strangeness)
		{
			warning("Resulting mesh elements may cross.  Consider modifying control points or vector, changing width, or changing resolution");
		}

		for (VMesh::Node::index_type idx=0;idx<fin_nodes.size();idx++)
		{

			mesh->vmesh()->add_point(fin_nodes[idx]);
		}

		VMesh::Node::index_type EE=0, EE1=1, EE2=2,EE3=3,EE4=4,EE5=5;
		VMesh::Node::index_type SE, SE1, SE2,SE3,SE4,SE5;

		int DN=5;

		SE=static_cast<VMesh::Node::index_type> (DN+1);
		SE1=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+1);
		SE2=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+2);
		SE3=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+3);
		SE4=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+4);
		SE5=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+5);

		for (VMesh::Node::index_type idx=0;idx<N-1;idx++)
		{


			EE1=static_cast<VMesh::Node::index_type> (static_cast<int> (EE)+1);

			VMesh::Node::array_type elem_nodes(4);


			//right side elements
			elem_nodes[0]=EE;
			elem_nodes[1]=EE1;
			elem_nodes[2]=EE2;
			elem_nodes[3]=SE;

			mesh->vmesh()->add_elem(elem_nodes);

			elem_nodes[0]=SE1;
			elem_nodes[1]=EE1;
			elem_nodes[2]=EE2;
			elem_nodes[3]=SE;

			mesh->vmesh()->add_elem(elem_nodes);

			elem_nodes[0]=SE1;
			elem_nodes[1]=EE2;
			elem_nodes[2]=EE3;
			elem_nodes[3]=EE1;

			mesh->vmesh()->add_elem(elem_nodes);

			elem_nodes[0]=SE;
			elem_nodes[1]=SE1;
			elem_nodes[2]=SE2;
			elem_nodes[3]=EE2;

			mesh->vmesh()->add_elem(elem_nodes);

			elem_nodes[0]=SE1;
			elem_nodes[1]=SE2;
			elem_nodes[2]=SE3;
			elem_nodes[3]=EE2;

			mesh->vmesh()->add_elem(elem_nodes);

			elem_nodes[0]=SE1;
			elem_nodes[1]=EE2;
			elem_nodes[2]=EE3;
			elem_nodes[3]=SE3;

			mesh->vmesh()->add_elem(elem_nodes);

			//left side elements
			elem_nodes[0]=EE;
			elem_nodes[1]=EE1;
			elem_nodes[2]=EE4;
			elem_nodes[3]=SE;

			mesh->vmesh()->add_elem(elem_nodes);

			elem_nodes[0]=SE1;
			elem_nodes[1]=EE1;
			elem_nodes[2]=EE4;
			elem_nodes[3]=SE;

			mesh->vmesh()->add_elem(elem_nodes);

			elem_nodes[0]=SE1;
			elem_nodes[1]=EE4;
			elem_nodes[2]=EE5;
			elem_nodes[3]=EE1;

			mesh->vmesh()->add_elem(elem_nodes);

			elem_nodes[0]=SE;
			elem_nodes[1]=SE1;
			elem_nodes[2]=SE4;
			elem_nodes[3]=EE4;

			mesh->vmesh()->add_elem(elem_nodes);

			elem_nodes[0]=SE1;
			elem_nodes[1]=SE4;
			elem_nodes[2]=SE5;
			elem_nodes[3]=EE4;

			mesh->vmesh()->add_elem(elem_nodes);

			elem_nodes[0]=SE1;
			elem_nodes[1]=EE4;
			elem_nodes[2]=EE5;
			elem_nodes[3]=SE5;

			mesh->vmesh()->add_elem(elem_nodes);

			EE=SE;
			EE1=SE1;
			EE2=SE2;
			EE3=SE3;
			EE4=SE4;
			EE5=SE5;



			SE=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+(DN)+1);
			SE1=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+1);
			SE2=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+2);
			SE3=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+3);
			SE4=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+4);
			SE5=static_cast<VMesh::Node::index_type> (static_cast<int> (SE)+5);

		}

		fi.make_double();
		ofield = CreateField(fi,mesh);

		send_output_handle("Output Field",ofield);

	}








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

void GenerateElectrodeImpl::get_centers(std::vector<Point>& p, std::vector<Point>& pp, double length, int resolution)
{
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  //This is only needed to get the positions from the widget
	get_points(p);
#endif
	std::vector<double> t(p.size());
	t[0]=0;

	for (size_t k=1; k<p.size(); k++)
	{
		t[k] = (p[k]-p[k-1]).length() + t[k-1];
	}

	std::vector<double> tt(resolution*(p.size()-1));
	for (size_t k=0; k< tt.size(); k++) tt[k] = static_cast<double>(k)*(length/(static_cast<double>(tt.size()-1)));

	impl::CalculateSpline(t,p,tt,pp);
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
