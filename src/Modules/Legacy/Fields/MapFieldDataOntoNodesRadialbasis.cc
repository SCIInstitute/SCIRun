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


#include <Modules/Legacy/Fields/MapFieldDataOntoNodesRadialbasis.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoNodes.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Math/MiscMath.h>
#include <Core/Logging/Log.h>
#include <vector>
#include <sstream>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;
using namespace SCIRun;

/// @todo: code cleanup needed
// documentation needed
// move calculation to algorithm class

/// @class MapFieldDataOntoNodesRadialbasis
/// @brief Maps data centered on the nodes to another set of nodes using a radial basis.

class MapFieldDataOntoNodesRadialbasisImpl
{
public:
  explicit MapFieldDataOntoNodesRadialbasisImpl(ModuleStateHandle state) : state_(state) {}
	bool radial_basis_func(FieldHandle& output, FieldHandle source, FieldHandle destination);
  bool interp_on_mesh(VMesh* points, VMesh* Cors, const DenseMatrix& coefs, FieldHandle output);
private:
  ModuleStateHandle state_;
};

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

void MapFieldDataOntoNodesRadialbasis::execute()
{
  auto source = getRequiredInput(Source);
  auto destination = getRequiredInput(Destination);

  if (needToExecute())
  {
    FieldHandle output;
    MapFieldDataOntoNodesRadialbasisImpl impl(get_state());
    impl.radial_basis_func(output, source, destination);
    sendOutput(Output, output);
  }
}

bool MapFieldDataOntoNodesRadialbasisImpl::radial_basis_func(FieldHandle& output, FieldHandle source, FieldHandle destination)
{
  auto cors = source->vmesh();
  auto points = destination->vmesh();

  FieldInformation fi(destination);
  FieldInformation fis(source);

  VMesh::Node::size_type num_cors, num_pts;
  VMesh::Node::iterator iti,itj;

  cors->size(num_cors);
  points->size(num_pts);

  DenseMatrix sigma(num_cors, num_cors);

  fi.set_data_type(fis.get_data_type());
  output = CreateField(fi, destination->mesh());

  //create the radial basis function
  for(int i=0;i<num_cors;++i)
  {
    for(int j=0;j<num_cors;++j)
    {
      iti=i;
      itj=j;

      Point Pc, Pp;
      cors->get_point(Pc,*(itj));
      cors->get_point(Pp,*(iti));

      auto xcomp = Pc.x() - Pp.x();
      auto ycomp = Pc.y() - Pp.y();
      auto zcomp = Pc.z() - Pp.z();

      auto mag = sqrt(pow(xcomp,2.0)+pow(ycomp,2.0)+pow(zcomp,2.0));
      if (mag == 0)
      {
        sigma(i,j) = sigma(j,i) = 0;
      }
      else
      {
        sigma(i,j) = sigma(j,i) = pow(mag,2.0)*log(mag);
      }
    }
  }

  //create the right side of the equation
  DenseMatrix rsideMat(num_cors, 1);

  auto ifield = source->vfield();
  //TODO: NOT how to iterate through nodes
  for(int i = 0; i < num_cors; ++i)
  {
    VMesh::Node::iterator it = i;
    double elec_val = 0.0;
    ifield->get_value(elec_val, *(it));
    rsideMat(i, 0) = elec_val;
  }

  //run SVD
  Eigen::JacobiSVD<DenseMatrix::EigenBase> svd_mat(sigma, Eigen::ComputeFullU | Eigen::ComputeFullV);

  DenseMatrix Um = svd_mat.matrixU();
  DenseMatrix Sm = svd_mat.singularValues();
  DenseMatrix Vm = svd_mat.matrixV();

  auto coefMat = Vm * (Um.transpose() * rsideMat).cwiseQuotient(Sm);

  //done with solve, make the new field
  return interp_on_mesh(points, cors, coefMat, output);
}

bool MapFieldDataOntoNodesRadialbasisImpl::interp_on_mesh(VMesh* points, VMesh* cors, const DenseMatrix& coefs, FieldHandle output)
{
  VMesh::Node::size_type num_cors, num_pts;
  VMesh::Node::iterator it,itp,iti,itj;

  auto ofield = output->vfield();

  if (ofield->is_nodata())
    return false;

  cors->size(num_cors);
  points->size(num_pts);

  double sumer=0.0;
  double sigma=0.0;
  bool max_dist = false;

  double guiMD = state_->getValue(Parameters::MaxDistance).toDouble();
  double guiOV = state_->getValue(Parameters::OutsideValue).toDouble();

  for (int i = 0; i < num_pts; ++i)
  {
    sumer = 0;
    max_dist = false;
    for (int j = 0; j<num_cors; ++j)
    {
      iti = i;
      itj = j;

      Point Pp, Pc;
      cors->get_point(Pc,*(itj));
      points->get_point(Pp,*(iti));

      auto xcomp = Pc.x()-Pp.x();
      auto ycomp = Pc.y()-Pp.y();
      auto zcomp = Pc.z()-Pp.z();

      auto mag = sqrt(pow(xcomp,2.0)+pow(ycomp,2.0)+pow(zcomp,2.0));

      if (mag > guiMD)
      {
        j = num_cors - 1;
        max_dist = true;
      }

      if (mag == 0)
      {
        sigma = 0;
      }
      else
      {
        #if 0 //else: //   std::cerr<<"Not yet implemented"<<std::endl;
        if(gui_value_.get() == "thin-plate-spline")
        #endif
        {
          sigma = pow(mag,2.0)*log(mag);
        }
      }
      sumer += coefs(j, 0) * sigma;
    }
    itp = i;

    if (max_dist)
    {
      sumer = guiOV;
    }
    ofield->set_value(sumer, *(itp));
  }

  return true;
}
