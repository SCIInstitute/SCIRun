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


#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateSignedDistanceField.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Thread/Parallel.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

class CalculateSignedDistanceFieldP : public Interruptible
{
  public:
    CalculateSignedDistanceFieldP(VMesh* imesh, VMesh* objmesh, VField*  ofield, const ProgressReporter* pr) :
      imesh(imesh), objmesh(objmesh), ofield(ofield), pr_(pr) {}

    CalculateSignedDistanceFieldP(VMesh* imesh, VMesh* objmesh, VField* objfield,
            VField* ofield, VField* vfield, const ProgressReporter* pr) :
      imesh(imesh), objmesh(objmesh), objfield(objfield), ofield(ofield), vfield(vfield), pr_(pr) {}

    void parallel(int proc, int nproc)
    {
      VMesh::size_type num_values = ofield->num_values();
      VMesh::size_type num_evalues = ofield->num_evalues();

      double val = 0.0;
      double epsilon = objmesh->get_epsilon();
      int cnt = 0;

      if (ofield->basis_order() == 0)
      {
        VMesh::Elem::index_type fidx, fidx_n;
        VMesh::Node::array_type nodes;
        VMesh::DElem::array_type delems;
        Vector n, k;
        Point n0,n1,n2;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);

        for (VMesh::Elem::index_type idx = start; idx < end; idx++)
        {
          checkForInterruption(this);
          Point p, p1, p2;
          imesh->get_center(p,idx);

          objmesh->find_closest_elem(val,p2,fidx,p);
          objmesh->get_nodes(nodes,fidx);
          objmesh->get_center(n0,nodes[0]);
          objmesh->get_center(n1,nodes[1]);
          objmesh->get_center(n2,nodes[2]);

          n = Cross(Vector(n1-n0),Vector(n2-n1));
          k = Vector(p-p2); k.normalize();

          double angle = Dot(n,k);
          if (angle < -epsilon)
          {
            val = -val;
          }
          else if (angle > epsilon)
          {
          }
          else
          {
            // trouble
            if (val != 0.0)
            {
               objmesh->get_delems(delems,fidx);
               double mindist = DBL_MAX;
               double dist;
               int edgeidx = 0;
               for (size_t r=0; r<delems.size();r++)
               {
                 objmesh->get_nodes(nodes,delems[r]);
                 objmesh->get_center(p1,nodes[0]);
                 objmesh->get_center(p2,nodes[1]);

                if (Dot(Vector(p-p2),Vector(p2-p1)) >= 0.0)
                {
                  Vector v = Vector(p-p2);
                  dist  = Dot(v,v);
                }
                else if (Dot(Vector(p-p1),Vector(p1-p2)) >= 0.0)
                {
                  Vector v = Vector(p-p1);
                  dist = Dot(v,v);
                }
                else
                {
                  Vector v1 = Vector(p1-p2);
                  Vector v = Vector(p-p2)-v1*(Dot(Vector(p-p2),v1)/Dot(v1,v1));
                  dist = Dot(v,v);
                }

                if (dist < mindist) { mindist = dist; edgeidx = r;}
              }
              objmesh->get_neighbor(fidx_n,fidx,delems[edgeidx]);
              objmesh->get_nodes(nodes,fidx);
              objmesh->get_center(n0,nodes[0]);
              objmesh->get_center(n1,nodes[1]);
              objmesh->get_center(n2,nodes[2]);
              n = Cross(Vector(n1-n0),Vector(n2-n1));
              k = Vector(p-p2);
              k.normalize();
              angle = Dot(n,k);
              if (angle < 0) val = -(val);
            }
          }
          checkForInterruption(this);
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 100) { pr_->update_progress_max(idx,end); cnt = 0; } }
        }
      }
      else if (ofield->basis_order() == 1)
      {
        VMesh::Elem::index_type fidx, fidx_n;
        VMesh::Node::array_type nodes;
        VMesh::DElem::array_type delems;

        Vector n, k;
        Point n0,n1,n2;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);

        for (VMesh::Node::index_type idx =start; idx <end; idx++)
        {
          checkForInterruption();
          Point p, p1, p2;
          imesh->get_center(p,idx);
          objmesh->find_closest_elem(val,p2,fidx,p);

          objmesh->get_nodes(nodes,fidx);
          objmesh->get_center(n0,nodes[0]);
          objmesh->get_center(n1,nodes[1]);
          objmesh->get_center(n2,nodes[2]);
          n = Cross(Vector(n1-n0),Vector(n2-n1));
          k = Vector(p-p2);
          k.normalize();
          double angle = Dot(n,k);
          if (angle < -epsilon)
          {
            val = -val;
          }
          else if (angle > epsilon)
          {
          }
          else
          {
            // trouble
            if (val != 0.0)
            {
               objmesh->get_delems(delems,fidx);
               double mindist = DBL_MAX;
               double dist;
               int edgeidx = 0;
               for (size_t r=0; r<delems.size();r++)
               {
                 objmesh->get_nodes(nodes,delems[r]);
                 objmesh->get_center(p1,nodes[0]);
                 objmesh->get_center(p2,nodes[1]);

                if (Dot(Vector(p-p2),Vector(p2-p1)) >= 0.0)
                {
                  Vector v = Vector(p-p2);
                  dist  = Dot(v,v);
                }
                else if (Dot(Vector(p-p1),Vector(p1-p2)) >= 0.0)
                {
                  Vector v = Vector(p-p1);
                  dist = Dot(v,v);
                }
                else
                {
                  Vector v1 = Vector(p1-p2);
                  Vector v = Vector(p-p2)-v1*(Dot(Vector(p-p2),v1)/Dot(v1,v1));
                  dist = Dot(v,v);
                }

                if (dist < mindist) { mindist = dist; edgeidx = r;}
              }
              objmesh->get_neighbor(fidx_n,fidx,delems[edgeidx]);
              objmesh->get_nodes(nodes,fidx);
              objmesh->get_center(n0,nodes[0]);
              objmesh->get_center(n1,nodes[1]);
              objmesh->get_center(n2,nodes[2]);
              n = Cross(Vector(n1-n0),Vector(n2-n1));
              k = Vector(p-p2);
              k.normalize();
              angle = Dot(n,k);
              if (angle < 0.0) val = -(val);
            }
          }
          checkForInterruption(this);
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 100) { pr_->update_progress_max(idx,end); cnt = 0; } }
        }
      }
      else if (ofield->basis_order() > 1)
      {
        VMesh::Elem::index_type fidx, fidx_n;
        VMesh::Node::array_type nodes;
        VMesh::DElem::array_type delems;

        Vector n, k;
        Point n0,n1,n2;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_evalues);

        for (VMesh::ENode::index_type idx=start; idx < end; idx++)
        {
          checkForInterruption(this);
          Point p, p1, p2;
          imesh->get_center(p,idx);
          objmesh->find_closest_elem(val,p2,fidx,p);

          objmesh->get_nodes(nodes,fidx);
          objmesh->get_center(n0,nodes[0]);
          objmesh->get_center(n1,nodes[1]);
          objmesh->get_center(n2,nodes[2]);
          n = Cross(Vector(n1-n0),Vector(n2-n1));
          k = Vector(p-p2);
          k.normalize();
          double angle = Dot(n,k);
          if (angle < -epsilon)
          {
            val = -val;
          }
          else if (angle > epsilon)
          {
          }
          else
          {
            // trouble
            if (val != 0.0)
            {
               objmesh->get_delems(delems,fidx);
               double mindist = DBL_MAX;
               double dist;
               int edgeidx = 0;
               for (size_t r=0; r<delems.size();r++)
               {
                 objmesh->get_nodes(nodes,delems[r]);
                 objmesh->get_center(p1,nodes[0]);
                 objmesh->get_center(p2,nodes[1]);

                if (Dot(Vector(p-p2),Vector(p2-p1)) >= 0.0)
                {
                  Vector v = Vector(p-p2);
                  dist  = Dot(v,v);
                }
                else if (Dot(Vector(p-p1),Vector(p1-p2)) >= 0.0)
                {
                  Vector v = Vector(p-p1);
                  dist = Dot(v,v);
                }
                else
                {
                  Vector v1 = Vector(p1-p2);
                  Vector v = Vector(p-p2)-v1*(Dot(Vector(p-p2),v1)/Dot(v1,v1));
                  dist = Dot(v,v);
                }

                if (dist < mindist) { mindist = dist; edgeidx = r;}
              }
              objmesh->get_neighbor(fidx_n,fidx,delems[edgeidx]);
              objmesh->get_nodes(nodes,fidx);
              objmesh->get_center(n0,nodes[0]);
              objmesh->get_center(n1,nodes[1]);
              objmesh->get_center(n2,nodes[2]);
              n = Cross(Vector(n1-n0),Vector(n2-n1));
              k = Vector(p-p2);
              k.normalize();
              angle = Dot(n,k);
              if (angle < 0.0) val = -(val);
            }
          }
          checkForInterruption(this);
          ofield->set_evalue(val,idx);
          if (proc == 0) { cnt++; if (cnt == 100) { pr_->update_progress_max(idx,end); cnt = 0; } }
        }
      }
    }

    void parallel2(int proc, int nproc)
    {
      VMesh::size_type num_values = ofield->num_values();
      VMesh::size_type num_evalues = ofield->num_evalues();

      double val = 0.0;
      double epsilon = objmesh->get_epsilon();
      int cnt = 0;

      if (ofield->basis_order() == 0)
      {
        VMesh::Elem::index_type fidx, fidx_n;
        VMesh::Node::array_type nodes;
        VMesh::DElem::array_type delems;
        VMesh::coords_type coords;

        Vector n, k;
        Point n0,n1,n2;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);

        for (VMesh::Elem::index_type idx = start; idx < end; idx++)
        {
          checkForInterruption(this);
          Point p, p1, p2;
          imesh->get_center(p,idx);

          objmesh->find_closest_elem(val,p2,coords,fidx,p);
          objmesh->get_nodes(nodes,fidx);
          objmesh->get_center(n0,nodes[0]);
          objmesh->get_center(n1,nodes[1]);
          objmesh->get_center(n2,nodes[2]);

          n = Cross(Vector(n1-n0),Vector(n2-n1));
          k = Vector(p-p2); k.normalize();

          double angle = Dot(n,k);
          if (angle < -epsilon)
          {
            val = -val;
          }
          else if (angle > epsilon)
          {
          }
          else
          {
            // trouble
            if (val != 0.0)
            {
               objmesh->get_delems(delems,fidx);
               double mindist = DBL_MAX;
               double dist;
               int edgeidx = 0;
               for (size_t r=0; r<delems.size();r++)
               {
                 objmesh->get_nodes(nodes,delems[r]);
                 objmesh->get_center(p1,nodes[0]);
                 objmesh->get_center(p2,nodes[1]);

                if (Dot(Vector(p-p2),Vector(p2-p1)) >= 0.0)
                {
                  Vector v = Vector(p-p2);
                  dist  = Dot(v,v);
                }
                else if (Dot(Vector(p-p1),Vector(p1-p2)) >= 0.0)
                {
                  Vector v = Vector(p-p1);
                  dist = Dot(v,v);
                }
                else
                {
                  Vector v1 = Vector(p1-p2);
                  Vector v = Vector(p-p2)-v1*(Dot(Vector(p-p2),v1)/Dot(v1,v1));
                  dist = Dot(v,v);
                }

                if (dist < mindist) { mindist = dist; edgeidx = r;}
              }
              objmesh->get_neighbor(fidx_n,fidx,delems[edgeidx]);
              objmesh->get_nodes(nodes,fidx);
              objmesh->get_center(n0,nodes[0]);
              objmesh->get_center(n1,nodes[1]);
              objmesh->get_center(n2,nodes[2]);
              n = Cross(Vector(n1-n0),Vector(n2-n1));
              k = Vector(p-p2);
              k.normalize();
              angle = Dot(n,k);
              if (angle < 0) val = -(val);
            }
          }
          checkForInterruption(this);
          ofield->set_value(val,idx);
          if (objfield->is_scalar())
          {
            double val;
            objfield->interpolate(val,coords,fidx);
            vfield->set_value(val,idx);
          }
          else if (objfield->is_vector())
          {
            Vector val;
            objfield->interpolate(val,coords,fidx);
            vfield->set_value(val,idx);
          }
          else if (objfield->is_tensor())
          {
            Tensor val;
            objfield->interpolate(val,coords,fidx);
            vfield->set_value(val,idx);
          }
          if (proc == 0) { cnt++; if (cnt == 100) { pr_->update_progress_max(idx,end); cnt = 0; } }
        }
      }
      else if (ofield->basis_order() == 1)
      {
        VMesh::Elem::index_type fidx, fidx_n;
        VMesh::Node::array_type nodes;
        VMesh::DElem::array_type delems;
        VMesh::coords_type coords;

        Vector n, k;
        Point n0,n1,n2;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);

        for (VMesh::Node::index_type idx =start; idx <end; idx++)
        {
          checkForInterruption(this);
          Point p, p1, p2;
          imesh->get_center(p,idx);
          objmesh->find_closest_elem(val,p2,coords,fidx,p);

          objmesh->get_nodes(nodes,fidx);
          objmesh->get_center(n0,nodes[0]);
          objmesh->get_center(n1,nodes[1]);
          objmesh->get_center(n2,nodes[2]);
          n = Cross(Vector(n1-n0),Vector(n2-n1));
          k = Vector(p-p2);
          k.normalize();
          double angle = Dot(n,k);
          if (angle < -epsilon)
          {
            val = -val;
          }
          else if (angle > epsilon)
          {
          }
          else
          {
            // trouble
            if (val != 0.0)
            {
               objmesh->get_delems(delems,fidx);
               double mindist = DBL_MAX;
               double dist;
               int edgeidx = 0;
               for (size_t r=0; r<delems.size();r++)
               {
                 Point p1, p2;
                 objmesh->get_nodes(nodes,delems[r]);
                 objmesh->get_center(p1,nodes[0]);
                 objmesh->get_center(p2,nodes[1]);

                if (Dot(Vector(p-p2),Vector(p2-p1)) >= 0.0)
                {
                  Vector v = Vector(p-p2);
                  dist  = Dot(v,v);
                }
                else if (Dot(Vector(p-p1),Vector(p1-p2)) >= 0.0)
                {
                  Vector v = Vector(p-p1);
                  dist = Dot(v,v);
                }
                else
                {
                  Vector v1 = Vector(p1-p2);
                  Vector v = Vector(p-p2)-v1*(Dot(Vector(p-p2),v1)/Dot(v1,v1));
                  dist = Dot(v,v);
                }

                if (dist < mindist) { mindist = dist; edgeidx = r;}
              }
              objmesh->get_neighbor(fidx_n,fidx,delems[edgeidx]);
              objmesh->get_nodes(nodes,fidx);
              objmesh->get_center(n0,nodes[0]);
              objmesh->get_center(n1,nodes[1]);
              objmesh->get_center(n2,nodes[2]);
              n = Cross(Vector(n1-n0),Vector(n2-n1));
              k = Vector(p-p2);
              k.normalize();
              angle = Dot(n,k);
              if (angle < 0.0) val = -(val);
            }
          }
          checkForInterruption(this);
          ofield->set_value(val,idx);
          if (objfield->is_scalar())
          {
            double val;
            objfield->interpolate(val,coords,fidx);
            vfield->set_value(val,idx);
          }
          else if (objfield->is_vector())
          {
            Vector val;
            objfield->interpolate(val,coords,fidx);
            vfield->set_value(val,idx);
          }
          else if (objfield->is_tensor())
          {
            Tensor val;
            objfield->interpolate(val,coords,fidx);
            vfield->set_value(val,idx);
          }
          if (proc == 0) { cnt++; if (cnt == 100) { pr_->update_progress_max(idx,end); cnt = 0; } }
        }
      }
      else if (ofield->basis_order() > 1)
      {
        VMesh::Elem::index_type fidx, fidx_n;
        VMesh::Node::array_type nodes;
        VMesh::DElem::array_type delems;
        VMesh::coords_type coords;

        Vector n, k;
        Point n0,n1,n2;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_evalues);

        for (VMesh::ENode::index_type idx=start; idx < end; idx++)
        {
          checkForInterruption(this);
          Point p, p1, p2;
          imesh->get_center(p,idx);
          objmesh->find_closest_elem(val,p2,coords,fidx,p);

          objmesh->get_nodes(nodes,fidx);
          objmesh->get_center(n0,nodes[0]);
          objmesh->get_center(n1,nodes[1]);
          objmesh->get_center(n2,nodes[2]);
          n = Cross(Vector(n1-n0),Vector(n2-n1));
          k = Vector(p-p2);
          k.normalize();
          double angle = Dot(n,k);
          if (angle < -epsilon)
          {
            val = -val;
          }
          else if (angle > epsilon)
          {
          }
          else
          {
            // trouble
            if (val != 0.0)
            {
               objmesh->get_delems(delems,fidx);
               double mindist = DBL_MAX;
               double dist;
               int edgeidx = 0;
               for (size_t r=0; r<delems.size();r++)
               {
                 objmesh->get_nodes(nodes,delems[r]);
                 objmesh->get_center(p1,nodes[0]);
                 objmesh->get_center(p2,nodes[1]);

                if (Dot(Vector(p-p2),Vector(p2-p1)) >= 0.0)
                {
                  Vector v = Vector(p-p2);
                  dist  = Dot(v,v);
                }
                else if (Dot(Vector(p-p1),Vector(p1-p2)) >= 0.0)
                {
                  Vector v = Vector(p-p1);
                  dist = Dot(v,v);
                }
                else
                {
                  Vector v1 = Vector(p1-p2);
                  Vector v = Vector(p-p2)-v1*(Dot(Vector(p-p2),v1)/Dot(v1,v1));
                  dist = Dot(v,v);
                }

                if (dist < mindist) { mindist = dist; edgeidx = r;}
              }
              objmesh->get_neighbor(fidx_n,fidx,delems[edgeidx]);
              objmesh->get_nodes(nodes,fidx);
              objmesh->get_center(n0,nodes[0]);
              objmesh->get_center(n1,nodes[1]);
              objmesh->get_center(n2,nodes[2]);
              n = Cross(Vector(n1-n0),Vector(n2-n1));
              k = Vector(p-p2);
              k.normalize();
              angle = Dot(n,k);
              if (angle < 0.0) val = -(val);
            }
          }
          checkForInterruption(this);
          ofield->set_evalue(val,idx);
          if (objfield->is_scalar())
          {
            double val;
            objfield->interpolate(val,coords,fidx);
            vfield->set_value(val,idx);
          }
          else if (objfield->is_vector())
          {
            Vector val;
            objfield->interpolate(val,coords,fidx);
            vfield->set_value(val,idx);
          }
          else if (objfield->is_tensor())
          {
            Tensor val;
            objfield->interpolate(val,coords,fidx);
            vfield->set_value(val,idx);
          }
          if (proc == 0) { cnt++; if (cnt == 100) { pr_->update_progress_max(idx,end); cnt = 0; } }
        }
      }
    }


    void range(int proc, int nproc,
               VMesh::index_type& start, VMesh::index_type& end,
               VMesh::size_type size)
    {
      VMesh::size_type m = size/nproc;
      start = proc*m;
      end = (proc+1)*m;
      if (proc == nproc-1) end = size;
    }

  private:
    VMesh*   imesh;
    VMesh*   objmesh;
    VField*  objfield;
    VField*  ofield;
    VField*  vfield;

    const ProgressReporter* pr_;
};

CalculateSignedDistanceFieldAlgo::CalculateSignedDistanceFieldAlgo()
{
  addParameter(OutputValueField, false);
}

bool
CalculateSignedDistanceFieldAlgo::run(FieldHandle input, FieldHandle object, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "CalculateDistanceField");

  if (!input)
  {
    error("No input field");
    return (false);
  }

  if (!object)
  {
    error("No object field");
    return (false);
  }

  if (!(object->vmesh()->is_surface()))
  {
    error("The object field needs to be surface");
    return (false);
  }

  // Determine output type
  FieldInformation fo(input);
  if (fo.is_nodata()) fo.make_lineardata();
  fo.make_double();

  output = CreateField(fo,input->mesh());

  if (!output)
  {
    error("Could not create output field");
    return (false);
  }

  VMesh* imesh = input->vmesh();
  VMesh* objmesh = object->vmesh();
  VField* ofield = output->vfield();
  ofield->resize_values();

  if (ofield->basis_order() > 2)
  {
    error("Cannot add distance data to field");
    return (false);
  }

  if (objmesh->num_nodes() == 0)
  {
    warning("Object Field does not contain any nodes, setting distance to maximum.");
    ofield->set_all_values(DBL_MAX);

    return (true);
  }

  objmesh->synchronize(Mesh::FIND_CLOSEST_ELEM_E|Mesh::EDGES_E);
  CalculateSignedDistanceFieldP palgo(imesh, objmesh, ofield, this);
  const int numThreads = Parallel::NumCores();
  auto task_i = [&palgo,numThreads](int i) { palgo.parallel(i, numThreads); };
  Parallel::RunTasks(task_i, numThreads);

  return (true);
}

bool
CalculateSignedDistanceFieldAlgo::run(FieldHandle input, FieldHandle object, FieldHandle& distance, FieldHandle& value) const
{
  ScopedAlgorithmStatusReporter asr(this, "CalculateDistanceField");

  if (!input)
  {
    error("No input field");
    return (false);
  }

  if (!object)
  {
    error("No object field");
    return (false);
  }

  if (!(object->vmesh()->is_surface()))
  {
    error("The object field needs to be surface");
    return (false);
  }

  // Determine output type
  FieldInformation fo(input);
  if (fo.is_nodata()) fo.make_lineardata();

  FieldInformation fb(object);

  if (fb.is_nodata())
  {
    error("Object field does not contain any values");
    return (false);
  }
  // Create Value mesh with same type as object type

  fo.set_data_type(fb.get_data_type());
  value = CreateField(fo,input->mesh());

  fo.make_double();
  distance = CreateField(fo,input->mesh());

  if (!distance)
  {
    error("Could not create output field");
    return (false);
  }

  if (!value)
  {
    error("Could not create output field");
    return (false);
  }

  VMesh* imesh = input->vmesh();
  VMesh* objmesh = object->vmesh();
  VField* objfield = object->vfield();

  VField* dfield = distance->vfield();
  dfield->resize_values();

  VField* vfield = value->vfield();
  vfield->resize_values();

  if (objmesh->num_nodes() == 0)
  {
    warning("Object Field does not contain any nodes, setting distance to maximum.");
    dfield->set_all_values(DBL_MAX);
    vfield->clear_all_values();

    return (true);
  }

  objmesh->synchronize(Mesh::FIND_CLOSEST_ELEM_E|Mesh::EDGES_E);

  if (distance->basis_order() > 2)
  {
    error("Cannot add distance data to field");
    return (false);
  }

  CalculateSignedDistanceFieldP palgo(imesh, objmesh, objfield, dfield, vfield, this);

  auto task_i = [&palgo](int i) { palgo.parallel2(i, Parallel::NumCores()); };
  Parallel::RunTasks(task_i, Parallel::NumCores());

  return (true);
}

const AlgorithmInputName CalculateSignedDistanceFieldAlgo::ObjectField("ObjectField");
const AlgorithmOutputName CalculateSignedDistanceFieldAlgo::SignedDistanceField("SignedDistanceField");
const AlgorithmOutputName CalculateSignedDistanceFieldAlgo::ValueField("ValueField");
const AlgorithmParameterName CalculateSignedDistanceFieldAlgo::OutputValueField("OutputValueField");

AlgorithmOutput CalculateSignedDistanceFieldAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  auto objectField = input.get<Field>(ObjectField);

  FieldHandle distance, value;
  if (get(OutputValueField).toBool())
  {
    if (!run(inputField, objectField, distance, value))
      THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");
  }
  else
  {
    if (!run(inputField, objectField, distance))
      THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");
  }

  AlgorithmOutput output;
  output[SignedDistanceField] = distance;
  output[ValueField] = value;
  return output;
}
