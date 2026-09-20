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


#include <Core/Algorithms/Legacy/Fields/GenerateElectrodeFromPointsAlgo.h>
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
ALGORITHM_PARAMETER_DEF(Fields, UseFieldNodes);

GenerateElectrodeFromPointsAlgo::GenerateElectrodeFromPointsAlgo()
{
  GenerateElectrodeFromPointsImpl impl_;
  
  addParameter(Parameters::ElectrodeLength, 0.1);
  addParameter(Parameters::ElectrodeThickness, 0.003);
  addParameter(Parameters::ElectrodeWidth, 0.02);
  addOption(Parameters::ElectrodeType,"wire","wire|planar");
  addOption(Parameters::ElectrodeProjection,"midway","positive|midway|negative");
  addParameter(Parameters::NumberOfControlPoints,5);
  addParameter(Parameters::ElectrodeResolution,10);
  addParameter(Parameters::UseFieldNodes,true);
}

// equivalent to the interp1 command in matlab.  uses the parameters p and t to perform a cubic spline interpolation pp in one direction.

bool GenerateElectrodeFromPointsImpl::CalculateSpline(std::vector<double>& t, std::vector<double>& x, std::vector<double>& tt, std::vector<double>& xx)
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

bool GenerateElectrodeFromPointsImpl::CalculateSpline(std::vector<double>& t, std::vector<Point>& p, std::vector<double>& tt, std::vector<Point>& pp)
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


std::vector<Point> GenerateElectrodeFromPointsAlgo::get_centers(std::vector<Point>& p) const
{
  std::vector<Point> pp;
  
  double length = get(Parameters::ElectrodeLength).toDouble();
  int resolution = get(Parameters::ElectrodeResolution).toInt();
  
  std::vector<double> t(p.size());
  t[0]=0;

  for (size_t k=1; k<p.size(); k++)
  {
      t[k] = (p[k]-p[k-1]).length() + t[k-1];
  }

  std::vector<double> tt(resolution*(p.size()-1));
  for (size_t k=0; k< tt.size(); k++) tt[k] = static_cast<double>(k)*(length/(static_cast<double>(tt.size()-1)));

  impl_ -> CalculateSpline(t,p,tt,pp);
  
  return pp;
}


FieldHandle GenerateElectrodeFromPointsAlgo::Make_Mesh_Wire(std::vector<Point>& final_points) const
{
    FieldInformation fi("TetVolMesh",0,"double");
    MeshHandle mesh = CreateMesh(fi);
    VMesh::Node::array_type nodes;
  
  double thickness = get(Parameters::ElectrodeThickness).toDouble();
  int resolution = get(Parameters::ElectrodeResolution).toInt();


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


FieldHandle GenerateElectrodeFromPointsAlgo::Make_Mesh_Planar(std::vector<Point>& final_points, Vector& direction) const
    {
        //-------make planar mesh---------

        FieldInformation fi("TetVolMesh",0,"double");
        MeshHandle mesh = CreateMesh(fi);
        VMesh::Node::array_type nodes;

        bool vect_strangeness=false;
        bool res_strangeness=false;

      auto projection = getOption(Parameters::ElectrodeProjection);
      double thickness = get(Parameters::ElectrodeThickness).toDouble();
      double width = get(Parameters::ElectrodeWidth).toDouble()/2.0;
      int resolution = get(Parameters::ElectrodeResolution).toInt();

        double aa, bb;

        if (projection=="positive")
        {
            aa=0;
            bb=1;
        }
        else if (projection=="midway")
        {
            aa=.5;
            bb=1;
        }
        else if (projection=="negative")
        {
            aa=0;
            bb=-1;
        }

        //cout <<"proj= "<<proj<<".  aa = "<<aa<<". bb "<<bb<<endl;


        Vector V1, V2, V, Vx, Vy, Vxold;


        size_t N=final_points.size();

        std::vector<Point> fin_nodes;


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

            if (Dot(V1,direction)>.8)
            {
                vect_strangeness=true;
                //std::cout <<"V1 . direction = "<<Dot(V1,direction)<<std::endl;
            }

            Vx=Cross(V1,direction);
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
             Vx=Cross(V2,direction);
             Vy=Cross(V2,Vy);
             }
             */

            Vx.normalize();
            Vy.normalize();

            Point pr=Point(p[k]+Vy*thickness*aa);

            fin_nodes.push_back(pr);
            fin_nodes.push_back(Point(pr-Vy*thickness*bb));
            Point srp=Point(pr+Vx*width);
            fin_nodes.push_back(srp);
            fin_nodes.push_back(Point(pr+Vx*width-Vy*thickness*bb));
            Point srn=Point(pr-Vx*width);
            fin_nodes.push_back(srn);
            fin_nodes.push_back(Point(pr-Vx*width-Vy*thickness*bb));

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
  
      FieldHandle ofield = CreateField(fi,mesh);

      return ofield;

    }



bool GenerateElectrodeFromPointsAlgo::runImpl(FieldHandle input, FieldHandle& outputField) const
{
    
  FieldInformation fis(input);
  std::vector<Point> orig_points;
  Vector direction;
  Vector defdir = Vector(-10, 10, 10);

  auto electrode_type = getOption(Parameters::ElectrodeType);
  
  if (!input)
  {
    error("input field required for GenerateElectrodeFromPointsAlgo");
  }

  VMesh* smesh = input->vmesh();
  VField* sfield = input->vfield();

  smesh->synchronize(Mesh::ELEM_LOCATE_E);

  VMesh::Node::size_type num_nodes = smesh->num_nodes();
  if (num_nodes > 50)
  {
    error("There are more input nodes than we have arbitrarily decided to allow.");
    return false;
  }

  VMesh::Node::array_type a;
  orig_points.resize(num_nodes);
  bool get_vect_direction=false;
  if (sfield->is_vector())
  {
    get_vect_direction=true;
//    std::cout <<"vector data detected"<<std::endl;
  }
  else
  {
    direction = defdir;
  }
//  std::cout <<"direction check -- "<<direction<<std::endl;
//  std::cout <<"defdir -- "<<defdir<<std::endl;

  for (VMesh::Node::index_type idx = 0; idx < num_nodes; idx++)
  {
    Point ap;
    smesh->get_center(ap, idx);

    orig_points[idx] = ap;
    
    // just get the first non-zero vector value
    if (get_vect_direction)
    {
      Vector d;
      sfield->get_value(d, idx);
//      std::cout <<"  the d = "<<d<<std::endl;
//      std::cout <<"  dnorm = "<<d.norm()<<std::endl;
//      std::cout <<"  the d = "<<d<<std::endl;
      if (d.norm()>0.0)
      {
        direction=d;
//        std::cout <<"  the d = "<<d<<std::endl;
//        std::cout <<"  dnorm = "<<d.norm()<<std::endl;
        get_vect_direction=false;
//        std::cout <<" vector found. direction = "<<direction<<std::endl;
//        std::cout <<"  the d = "<<d<<std::endl;
//        direction.normalize();
//        std::cout <<" vector found. direction = "<<direction<<std::endl;
      }
    }
  }
  
  std::vector<Point> final_points = get_centers(orig_points);

  if (electrode_type == "wire")
    outputField = Make_Mesh_Wire(final_points);
  
  if (electrode_type == "planar")
  {
    std::cout <<" direction for mesh = "<<direction<<std::endl;
    outputField = Make_Mesh_Planar(final_points, direction);
  }
  
  return true;
    
  
}

AlgorithmOutput GenerateElectrodeFromPointsAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
    

  FieldHandle outputField;
  if (!runImpl(inputField, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[ElectrodeMesh] = outputField;
  return output;
}


const AlgorithmOutputName GenerateElectrodeFromPointsAlgo::ElectrodeMesh("ElectrodeMesh");
 
