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

#include <Core/Algorithms/Fields/Mapping/FindClosestNodesFromPointField.h>

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


class FindClosestNodesFromPointFieldPAlgo
{
  public:
    FindClosestNodesFromPointFieldPAlgo() :
      barrier_("FindClosestNodesFromPointFieldPAlgo Barrier") {}

    void parallel(int proc, int nproc);

    VField* sfield_;
    VField* dfield_;
    VField* ofield_;
    VMesh*  smesh_;
    VMesh*  dmesh_;
    VMesh*  omesh_;

    AlgoBase*  algo_;

    std::vector<double> mindist_array_;


  private:
    Barrier  barrier_;
};

void
FindClosestNodesFromPointFieldPAlgo::parallel(int proc, int nproc)
{
  // Determine which ones to run
  VField::size_type num_values = dfield_->num_values();
  VField::size_type localsize = num_values/nproc;
  VField::index_type start = localsize*proc;
  VField::index_type end = localsize*(proc+1);
  if (proc == nproc-1) end = num_values;
  if (proc == 0) mindist_array_.resize(num_values,DBL_MAX);

  barrier_.wait(nproc);


  VMesh::size_type num_snodes = smesh_->num_nodes();

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

      double mindist = DBL_MAX;

      Point d = v.asPoint();
      VMesh::Node::index_type didx = -1;
      for (VMesh::Node::index_type sidx = 0 ; sidx < num_snodes; sidx++)
      {
        Point c;
        smesh_->get_center(c,sidx);
        double dist = (c-d).length();
        if ( dist < mindist)
        {
          mindist = dist;
          didx = sidx;
        }
      }

      ofield_->set_value(didx,idx);
      mindist_array_[idx] = mindist;
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

      double mindist = DBL_MAX;

      Point d = v.asPoint();
      VMesh::Node::index_type didx = -1;
      for (VMesh::Node::index_type sidx = 0 ; sidx < num_snodes; sidx++)
      {
        Point c;
        smesh_->get_center(c,sidx);
        double dist = (c-d).length();
        if ( dist < mindist)
        {
          mindist = dist;
          didx = sidx;
        }
      }

      ofield_->set_value(didx,idx);
      mindist_array_[idx] = mindist;
    }
  }

  barrier_.wait(nproc);
}


bool
FindClosestNodesFromPointFieldAlgo::
run(FieldHandle source, FieldHandle destination,
    FieldHandle& output)
{
  algo_start("FindClosestNodesFromPointField");

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

  fid.set_data_type("int");
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

  FindClosestNodesFromPointFieldPAlgo algo;
  algo.sfield_ = sfield;
  algo.dfield_ = dfield;
  algo.ofield_ = ofield;
  algo.smesh_ = smesh;
  algo.dmesh_ = dmesh;
  algo.omesh_ = omesh;
  algo.algo_ = this;

  int np = Thread::numProcessors();
  Thread::parallel(&algo,&FindClosestNodesFromPointFieldPAlgo::parallel,np,np);

  // Filter not used values

  VMesh::size_type num_nodes = smesh->num_nodes();
  VField::size_type num_values = ofield->num_values();
  for (VField::index_type idx=0; idx< num_nodes; idx++)
  {
    double mindist = DBL_MAX;
    index_type keep_index = 0;
    for (VMesh::index_type ii=0; ii<num_values; ii++)
    {
      index_type nidx;
      ofield->get_value(nidx,ii);

      if (nidx == idx && algo.mindist_array_[ii] < mindist)
      {
        mindist = algo.mindist_array_[ii];
        keep_index = ii;
      }
    }

    for (VMesh::index_type ii=0; ii<num_values; ii++)
    {
      index_type nidx;
      ofield->get_value(nidx,ii);
      if (nidx == idx && keep_index != ii)
        ofield->set_value(-1,ii);
    }
  }


  ofield->copy_properties(dfield);

  algo_end(); return (true);
}


} // end namespace SCIRunAlgo
