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


#include <Core/Thread/Thread.h>
#include <Core/Thread/Barrier.h>

#include <Core/Algorithms/Fields/Mapping/MapFromPointField.h>

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>

// for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;


//------------------------------------------------------------
// Algorithm - get interpolated data


class MapFromPointFieldPAlgo
{
  public:
    MapFromPointFieldPAlgo() :
      barrier_(" MapFromPointFieldPAlgo Barrier") {}

    void parallel(int proc, int nproc);

    VField* sfield_;
    VField* dfield_;
    VField* ofield_;
    VMesh*  smesh_;
    VMesh*  dmesh_;
    VMesh*  omesh_;

    double     maxdist_;
    AlgoBase*  algo_;

  private:
    Barrier  barrier_;
};

void
MapFromPointFieldPAlgo::parallel(int proc, int nproc)
{
  // Determine which ones to run
  VField::size_type num_values = dfield_->num_values();
  VField::size_type localsize = num_values/nproc;
  VField::index_type start = localsize*proc;
  VField::index_type end = localsize*(proc+1);
  if (proc == nproc-1) end = num_values;

  barrier_.wait(nproc);

  int cnt = 0;
  if (dfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::Elem::index_type didx;

    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      Vector v;
      double val;
      dfield_->get_value(v,idx);
      sfield_->interpolate(val,v.asPoint());
      ofield_->set_value(val,idx);
    }
  }
  else if (dfield_->basis_order() == 1)
  {
    Point p, r;
    VMesh::Node::index_type didx;

    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      Vector v;
      double val;
      dfield_->get_value(v,idx);
      sfield_->interpolate(val,v.asPoint());
      ofield_->set_value(val,idx);
    }
  }

  barrier_.wait(nproc);
}


bool
MapFromPointFieldAlgo::
run(FieldHandle source, FieldHandle destination,
    FieldHandle& output)
{
  algo_start("MapFromPointField");

  if (source.get_rep() == 0)
  {
    error("No source field");
    algo_end(); return (false);
  }

  if (destination.get_rep() == 0)
  {
    error("No destination field");
    algo_end(); return (false);
  }

  FieldInformation fis(source);
  FieldInformation fid(destination);

  fid.set_data_type(fis.get_data_type());
  output = CreateField(fid,destination->mesh());

  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);
  }

  // Determine output type

  VMesh* smesh = source->vmesh();
  VMesh* dmesh = destination->vmesh();
  VMesh* omesh = output->vmesh();

  VField* sfield = source->vfield();
  VField* dfield = destination->vfield();
  VField* ofield = output->vfield();

  // Make sure output field is all empty and of right size
  ofield->resize_values();
  ofield->clear_all_values();
  ofield->set_all_values(get_scalar("default_value"));

  int dbasis_order = dfield->basis_order();

  if (dbasis_order < 0)
  {
    error("Destination field basis order needs to constant or linear");
    algo_end(); return (false);
  }

  if (!(dfield->is_vector()))
  {
    error("Destination field needs to have the point locations of where to look for the data");
    algo_end(); return (false);
  }

  if (smesh->num_elems() > 0)
  {
    smesh->synchronize(Mesh::ELEM_LOCATE_E);
  }
  else
  {
    error("Source does not have any elements, hence one cannot interpolate data in this field");
    error("Use a closestdata interpolation scheme for this data");
    algo_end(); return (false);
  }

  double maxdist = get_scalar("max_distance");

  MapFromPointFieldPAlgo algo;
  algo.sfield_ = sfield;
  algo.dfield_ = dfield;
  algo.ofield_ = ofield;
  algo.smesh_ = smesh;
  algo.dmesh_ = dmesh;
  algo.omesh_ = omesh;

  algo.maxdist_ = maxdist;
  algo.algo_ = this;

  int np = Thread::numProcessors();
  Thread::parallel(&algo,&MapFromPointFieldPAlgo::parallel,np,np);

  ofield->copy_properties(dfield);

  algo_end(); return (true);
}


} // end namespace SCIRunAlgo
