/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#include <Modules/Legacy/Fields/MapFieldDataOntoNodesRadialbasis.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoNodes.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Geometry/Vector.h>
#include <Core/Math/MiscMath.h>
#include <vector>
#include <sstream>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

/// @todo: code cleanup needed
// documentation needed
// move calculation to algorithm class

/// @class MapFieldDataOntoNodesRadialbasis
/// @brief Maps data centered on the nodes to another set of nodes using a radial basis.

/*
class MapFieldDataOntoNodesRadialbasis : public Module {
  public:
    MapFieldDataOntoNodesRadialbasis(GuiContext*);
    virtual ~MapFieldDataOntoNodesRadialbasis() {}

    virtual void execute();

  private:
    GuiString gui_quantity_;
    GuiString gui_value_;
    GuiDouble gui_outside_value_;
    GuiDouble gui_max_distance_;

	bool radial_basis_func(VMesh* Cors, VMesh* points, FieldHandle& output, FieldHandle& input_s, FieldHandle& input_d);
  bool interp_on_mesh(VMesh* points, VMesh* Cors, std::vector<double>& coefs,  FieldHandle& output);
    //SCIRunAlgo::MapFieldDataOntoNodesRadialbasisAlgo algo_;
};
*/

MODULE_INFO_DEF(MapFieldDataOntoNodesRadialbasis, ChangeFieldData, SCIRun)

MapFieldDataOntoNodesRadialbasis::MapFieldDataOntoNodesRadialbasis() : Module(staticInfo_)
{
  INITIALIZE_PORT(Source);
  INITIALIZE_PORT(Destination);
  INITIALIZE_PORT(Output);
}

void MapFieldDataOntoNodesRadialbasis::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::OutsideValue, 0.0);
  state->setValue(Parameters::MaxDistance, std::numeric_limits<double>::max());
  state->setValue(Parameters::InterpolationModel, std::string("thin-plate-spline"));
  state->setValue(Parameters::Quantity, std::string("value"));
}

void
MapFieldDataOntoNodesRadialbasis::execute()
{
  FieldHandle source, destination, output;

  get_input_handle("Source",source,true);
  get_input_handle("Destination",destination,true);


  if (inputs_changed_ || !oport_cached("Output") ||
    gui_quantity_.changed() || gui_value_.changed() ||
    gui_outside_value_.changed() || gui_max_distance_.changed())
  {
    update_state(Executing);


      if (source.get_rep() == 0)
      {
        error("No source field");
        return;
      }

      if (destination.get_rep() == 0)
      {
        error("No destination field");
        return;
      }



      VMesh* imesh = source->vmesh();
      VMesh* dmesh = destination->vmesh();
      radial_basis_func(imesh, dmesh, output,source,destination);

      send_output_handle("Output",output,true);
  }
}


bool
MapFieldDataOntoNodesRadialbasis:: radial_basis_func(VMesh* Cors, VMesh* points, FieldHandle& output, FieldHandle& input_s, FieldHandle& input_d)
{
    FieldHandle input_cp;
    FieldInformation fi(input_d);
    FieldInformation fis(input_s);

    input_cp = input_s;
    input_cp.detach();
    input_cp->mesh_detach();


    VMesh::Node::size_type num_cors, num_pts;
    VMesh::Node::iterator iti,itj;
    VMesh::Node::iterator it;
    SCIRun::Point Pc,Pp;


    Cors->size(num_cors);
    points->size(num_pts);

    double xcomp=0.0,ycomp=0.0,zcomp=0.0,mag=0.0;

    DenseMatrixHandle Sigma = new DenseMatrix(num_cors,num_cors);

    VField* ifield = input_cp->vfield();
    fi.set_data_type(fis.get_data_type());
    output = CreateField(fi,input_d->mesh());


    if (output.get_rep() == 0)
    {
      error("Could not allocate output field");
      return (false);
    }

    double elec_val=0.0;
    double temp=0.0;

    //create the radial basis function
    for(int i=0;i<num_cors;++i)
    {
      for(int j=0;j<num_cors;++j)
      {
        iti=i;
        itj=j;

        Cors->get_point(Pc,*(itj));
        Cors->get_point(Pp,*(iti));

        xcomp = Pc.x() - Pp.x();
        ycomp = Pc.y() - Pp.y();
        zcomp = Pc.z() - Pp.z();

        mag = sqrt(pow(xcomp,2.0)+pow(ycomp,2.0)+pow(zcomp,2.0));
        if(mag==0)
        {
          Sigma->put(i,j,0);
          Sigma->put(j,i,0);
        }
        else
        {
          temp = pow(mag,2.0)*log(mag);

          Sigma->put(i,j,temp);
          Sigma->put(j,i,temp);
        }
      }
    }


    //create the right side of the equation
    std::vector<double> coefs;//(3*num_cors1+9);
    std::vector<double> rside;//(3*num_cors1+9);


    for(int i=0;i<num_cors;++i)
    {
      it=i;
      ifield->get_value(elec_val,*(it));
      rside.push_back(elec_val);
    }


    //Create sparse matrix for sigmas of svd
    int m = num_cors;
    int n = num_cors;

    SparseRowMatrixHandle Sm;

    //create the U and V matrix
    DenseMatrixHandle Um = new DenseMatrix(m,n);
    DenseMatrixHandle Vm = new DenseMatrix(n,n);

    //run SVD
    try
    {
      LinearAlgebra::svd(*Sigma, *Um, Sm, *Vm);
    }
    catch (const SCIRun::Exception& exception)
    {
      std::ostringstream oss;
      oss << "Caught exception: " << exception.type() << " " << exception.message();
      error(oss.str());
      return (false);
    }

    //Make more storage for the solving the linear least squares
    DenseMatrixHandle RsideMat = new DenseMatrix(m,1);
    DenseMatrixHandle CMat = new DenseMatrix(n,1);
    DenseMatrixHandle YMat = new DenseMatrix(n,1);
    DenseMatrixHandle CoefMat = new DenseMatrix(n,1);

    for(int loop=0;loop<n;++loop)
    {
      RsideMat->put(loop,0,rside[loop]);
    }


  //c=trans(Um)*rside;
  Mult_trans_X(*CMat, *Um,*RsideMat);


  for(int k=0;k<n;k++)
  {
     YMat->put(k,0,CMat->get(k,0)/Sm->get(k,k));
  }

  Mult_trans_X(*CoefMat, *Vm, *YMat);

  for(int p=0;p<n;p++)
  {
    coefs.push_back(CoefMat->get(p,0));
  }

  //done with solve, make the new field
  interp_on_mesh(points, Cors, coefs, output);

  return true;
}

bool
MapFieldDataOntoNodesRadialbasis::interp_on_mesh(VMesh* points, VMesh* Cors, std::vector<double>& coefs, FieldHandle& output)
{
  VMesh::Node::size_type num_cors, num_pts;
  VMesh::Node::iterator it,itp,iti,itj;
  SCIRun::Point P,Pp,Pc;

  VField* ofield = output->vfield();

  Cors->size(num_cors);
  points->size(num_pts);

  double xcomp=0.0, ycomp=0.0, zcomp=0.0, mag=0.0;
  double sumer=0.0;
  double sigma=0.0;
  int max_dist=0;

  double guiMD = (gui_max_distance_.get());
  double guiOV = (gui_outside_value_.get());

  for(int i=0; i< num_pts;++i)
  {
    sumer=0;
    max_dist=0.0;
    for(int j=0; j<num_cors; ++j)
    {

      iti=i;
      itj=j;

      Cors->get_point(Pc,*(itj));
      points->get_point(Pp,*(iti));

      xcomp=Pc.x()-Pp.x();
      ycomp=Pc.y()-Pp.y();
      zcomp=Pc.z()-Pp.z();

      mag=sqrt(pow(xcomp,2.0)+pow(ycomp,2.0)+pow(zcomp,2.0));

      if(mag > guiMD)
      {
        j=(num_cors-1);
        max_dist=1;
      }

      if(mag==0)
      {
        sigma=0;
      }
      else
      {

        if(gui_value_.get() == "thin-plate-spline")
        {
          sigma=pow(mag,2.0)*log(mag);
        }
        else
        {
          std::cerr<<"Not yet implemented"<<std::endl;
          j=num_cors-1;
          i=num_pts-1;
        }

      }
      sumer+=coefs[j]*sigma;

    }
    itp=i;
    if( max_dist == 0)
    {
      ofield->set_value(sumer,*(itp));
    }
    else
    {
      sumer = guiOV;
      ofield->set_value(sumer,*(itp));
    }
  }

  return true;
}


} // End namespace SCIRun
