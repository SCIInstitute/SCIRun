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


#include <Core/Algorithms/Legacy/Fields/GenerateElectrodeAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>


using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;

ALGORITHM_PARAMETER_DEF(Fields, ElectrodeLength);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeThickness);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeWidth);
ALGORITHM_PARAMETER_DEF(Fields, NumberOfControlPoints);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeType);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeResolution);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeProjection);
ALGORITHM_PARAMETER_DEF(Fields, MoveAll);
ALGORITHM_PARAMETER_DEF(Fields, UseFieldNodes);

GenerateElectrodeAlgo::GenerateElectrodeAlgo()
{
  addParameter(Parameters::ElectrodeLength, 0.1);
  addParameter(Parameters::ElectrodeThickness, 0.003);
  addParameter(Parameters::ElectrodeWidth, 0.02);
  addOption(Parameters::ElectrodeType,"wire","wire|planar");
  addOption(Parameters::ElectrodeProjection,"midway","positive|midway|negative");
  addParameter(Parameters::NumberOfControlPoints,5);
  addParameter(Parameters::ElectrodeResolution,10);
  addParameter(Parameters::UseFieldNodes,true);
  addParameter(Parameters::MoveAll,false);
  
}

//namespace detail
//{
//class GenerateElectrodeAlgoF {
//  public:
//    typedef std::pair<double, VMesh::Elem::index_type> weight_type;
//    typedef std::vector<weight_type> table_type;
//
//    bool build_table(VMesh *mesh, VField* vfield,
//                     std::vector<weight_type> &table,
//                     std::string& method);
//
//    static bool
//    weight_less(const weight_type &a, const weight_type &b)
//    {
//      return (a.first < b.first);
//    }
//};

bool
GenerateElectrodeAlgo::build_table(VMesh *vmesh,
                                                VField* vfield,
                                                std::vector<weight_type> &table,
                                                std::string& method)
{
  VMesh::size_type num_elems = vmesh->num_elems();

  long double sum = 0.0;
  for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
  {
    double elemsize = 0.0;
    if (method == "impuni")
    { // Size of element * data at element.
      Point p;
      vmesh->get_center(p, idx);
      if (vfield->is_vector())
      {
        Vector v;
        if (vfield->interpolate(v, p))
        {
          elemsize = v.length() * vmesh->get_size(idx);
        }
      }
      if (vfield->is_scalar())
      {
        double d;
        if (vfield->interpolate(d, p) && d > 0.0)
        {
          elemsize = d * vmesh->get_size(idx);
        }
      }
    }
    else if (method == "impscat")
    { // data at element
      Point p;
      vmesh->get_center(p, idx);
      if (vfield->is_vector())
      {
        Vector v;
        if (vfield->interpolate(v, p))
        {
          elemsize = v.length();
        }
      }
      if (vfield->is_scalar())
      {
        double d;
        if (vfield->interpolate(d, p) && d > 0.0)
        {
          elemsize = d;
        }
      }
    }
    else if (method == "uniuni")
    { // size of element only
      elemsize = vmesh->get_size(idx);
    }
    else if (method == "uniscat")
    {
      elemsize = 1.0;
    }

    if (elemsize > 0.0)
    {
      sum += elemsize;
      table.push_back(weight_type(sum, idx));
    }
  }
  if (table.size() > 0)
  {
    return (true);
  }

  return (false);
}



// equivalent to the interp1 command in matlab.  uses the parameters p and t to perform a cubic spline interpolation pp in one direction.

bool GenerateElectrodeAlgo::CalculateSpline(std::vector<double>& t, std::vector<double>& x, std::vector<double>& tt, std::vector<double>& xx)
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

bool GenerateElectrodeAlgo::CalculateSpline(std::vector<double>& t, std::vector<Point>& p, std::vector<double>& tt, std::vector<Point>& pp)
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

void GenerateElectrodeAlgo::get_centers(std::vector<Point>& p, std::vector<Point>& pp, double length, int resolution) const
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

    CalculateSpline(t,p,tt,pp);
}


FieldHandle GenerateElectrodeAlgo::Make_Mesh_Wire(std::vector<Point>& final_points, double thickness, int resolution) const
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

#endif


bool GenerateElectrodeAlgo::runImpl(FieldHandle input, FieldHandle& outputField, FieldHandle& outputPoints) const
{
    
    FieldInformation fis(input);
    std::vector<Point> orig_points;
    Vector direction;
    Vector defdir = Vector(-10, 10, 10);

    auto electrode_type = getOption(Parameters::ElectrodeType);
    
    if (input
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      && (use_field == 1) && (moveto == "default" || widget_.size() == 0 || inputs_changed_)
  #endif
      )
    {
      VMesh* smesh = input->vmesh();

      smesh->synchronize(Mesh::ELEM_LOCATE_E);

      VMesh::Node::size_type num_nodes = smesh->num_nodes();
      if (num_nodes > 50)
      {
        error("Why would you want to use that many nodes to make an electrode?  Do you want to crash you system?  That's way to many.");
        return false;
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
    return false;
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

  if (Previous_points_.size() < 3)
  {
    Previous_points_ = orig_points;
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

  Previous_points_ = orig_points;

  FieldInformation pi("PointCloudMesh", 0, "double");
  MeshHandle pmesh = CreateMesh(pi);

  for (VMesh::Node::index_type idx = 0; idx < orig_points.size(); idx++) pmesh->vmesh()->add_point(orig_points[idx]);

  //TODO: copy this here since widgets don't exist yet
  points = orig_points;

  pi.make_double();
  outputPoints = CreateField(pi, pmesh);
    
  get_centers(points, final_points,
      get(Parameters::ElectrodeLength).toDouble(),
      get(Parameters::ElectrodeResolution).toInt());

    if (electrode_type == "wire")
      outputField = Make_Mesh_Wire(final_points,
        get(Parameters::ElectrodeThickness).toDouble(),
        get(Parameters::ElectrodeResolution).toInt()));

  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (electrode_type == "planar")
        outputField = Make_Mesh_Planar(final_points, ofield, direction);
  #endif

    return true;
    
  
}

AlgorithmOutput GenerateElectrodeAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
    

  FieldHandle outputField;
  FieldHandle outputPoints;
  if (!runImpl(inputField, outputField, outputPoints))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[ControlPoints] = outputPoints;
  output[ElectrodeMesh] = outputField;
  return output;
}

