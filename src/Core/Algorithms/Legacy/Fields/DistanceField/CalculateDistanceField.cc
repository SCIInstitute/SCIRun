/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Fields/DistanceField/CalculateDistanceField.h>

#include <Core/Datatypes/FieldInformation.h>
#include <Core/Thread/Thread.h>

#include <float.h>

// for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;


class CalculateDistanceFieldP {
  public:
    CalculateDistanceFieldP(VMesh* imesh, VMesh* objmesh, VField*  ofield, AlgoBase* algo) :
      imesh(imesh), objmesh(objmesh), ofield(ofield), algo_(algo) {}

    CalculateDistanceFieldP(VMesh* imesh, VMesh* objmesh, VField* objfield, VField*  ofield, VField* vfield, AlgoBase* algo) :
      imesh(imesh), objmesh(objmesh), objfield(objfield), ofield(ofield), vfield(vfield), algo_(algo)  {}

    void parallel(int proc, int nproc)
    {
      VMesh::size_type num_values = ofield->num_values();
      VMesh::size_type num_evalues = ofield->num_evalues();
    
      double max = DBL_MAX;
      if (algo_->get_bool("truncate"))
      {
        max = algo_->get_scalar("truncate_distance");
      }
      
      double val = 0.0;
      int cnt = 0;

      if (ofield->basis_order() == 0)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);
        
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          Point p, p2;
          imesh->get_center(p,idx);
          if(!(objmesh->find_closest_elem(val,p2,fidx,p,max))) val = max;
          ofield->set_value(val,idx);
          
          if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress(idx,end); cnt = 0; } }
        }
      }
      else if (ofield->basis_order() == 1)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);

        for (VMesh::Node::index_type idx=start; idx<end; idx++)
        {
          Point p, p2;
          imesh->get_center(p,idx);
          if(!(objmesh->find_closest_elem(val,p2,fidx,p,max))) val = max;
          ofield->set_value(val,idx);
          
          if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress(idx,end); cnt = 0; } }
        }  
      }
      else if (ofield->basis_order() > 1)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_evalues);

        for (VMesh::ENode::index_type idx=start; idx<end; idx++)
        {
          Point p, p2;
          imesh->get_center(p,idx);
          if(!(objmesh->find_closest_elem(val,p2,fidx,p,max))) val = max;
          ofield->set_value(val,idx);
          
          if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress(idx,end); cnt = 0; } }
        }  
      }    
    }

    void parallel2(int proc, int nproc)
    {
      VMesh::size_type num_values = ofield->num_values();
      VMesh::size_type num_evalues = ofield->num_evalues();
    
      if (algo_->get_bool("truncate"))
      {
        // Cannot do both at the same time
        if (proc == 0) algo_->warning("Closest value has been requested, disabling truncated distance map.");
      }
      
      double val = 0.0;
      int cnt = 0;

      if (ofield->basis_order() == 0)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        VMesh::coords_type coords;
        Point p, p2;
        range(proc,nproc,start,end,num_values);
        
        if (objfield->is_scalar())
        {
          double scalar;
          
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(scalar,coords,fidx);
            vfield->set_value(scalar,idx);
            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress(idx,end); cnt = 0; } }
          }
        }
        else if (objfield->is_vector())
        {
          Vector vec;
          
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(vec,coords,fidx);
            vfield->set_value(vec,idx);
            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress(idx,end); cnt = 0; } }
          }
        }
        else if (objfield->is_tensor())
        {
          Tensor tensor;
          
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(tensor,coords,fidx);
            vfield->set_value(tensor,idx);
            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress(idx,end); cnt = 0; } }
          }
        }        
      }
      else if (ofield->basis_order() == 1)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        VMesh::coords_type coords;
        range(proc,nproc,start,end,num_values);
        Point p, p2;
        
        if (objfield->is_scalar())
        {
          double scalar;
          
          for (VMesh::Node::index_type idx=start; idx<end; idx++)
          {
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(scalar,coords,fidx);
            vfield->set_value(scalar,idx);
            
            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress(idx,end); cnt = 0; } }
          }
        }
        else if (objfield->is_vector())
        {
          Vector vec;
          
          for (VMesh::Node::index_type idx=start; idx<end; idx++)
          {
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(vec,coords,fidx);
            vfield->set_value(vec,idx);
            
            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress(idx,end); cnt = 0; } }
          }
        }
        else if (objfield->is_tensor())
        {
          Tensor tensor;
          
          for (VMesh::Node::index_type idx=start; idx<end; idx++)
          {
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(tensor,coords,fidx);
            vfield->set_value(tensor,idx);
            
            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress(idx,end); cnt = 0; } }
          }
        }
      }
      else if (ofield->basis_order() > 1)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        VMesh::coords_type coords;
        range(proc,nproc,start,end,num_evalues);
        Point p, p2;

        if (objfield->is_scalar())
        {
          double scalar;
          for (VMesh::ENode::index_type idx=start; idx<end; idx++)
          {
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(scalar,coords,fidx);
            vfield->set_value(scalar,idx);
            
            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress(idx,end); cnt = 0; } }
          }  
        }
        else if (objfield->is_vector())
        {
          Vector vec;
          for (VMesh::ENode::index_type idx=start; idx<end; idx++)
          {
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(vec,coords,fidx);
            vfield->set_value(vec,idx);
            
            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress(idx,end); cnt = 0; } }
          }  
        }
        else if (objfield->is_tensor())
        {
          Tensor tensor;
          for (VMesh::ENode::index_type idx=start; idx<end; idx++)
          {
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(tensor,coords,fidx);
            vfield->set_value(tensor,idx);
            
            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress(idx,end); cnt = 0; } }
          }  
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
    AlgoBase* algo_;
};


bool
CalculateDistanceFieldAlgo::
run(FieldHandle input, FieldHandle object, FieldHandle& output)
{
  algo_start("CalculateDistanceField");
  
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  if (object.get_rep() == 0)
  {
    error("No object field");
    algo_end(); return (false);
  }

  // Determine output type
  FieldInformation fo(input);
  if (fo.is_nodata()) fo.make_lineardata();

  if (check_option("basistype","linear")) fo.make_lineardata();
  if (check_option("basistype","constant")) fo.make_constantdata();

  fo.set_data_type(get_option("datatype"));

  fo.make_double();
  output = CreateField(fo,input->mesh());
  
  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return (false);
  }
  
  VMesh* imesh = input->vmesh();
  VMesh* objmesh = object->vmesh();
  VField* ofield = output->vfield();
  ofield->resize_values();
 
  if (imesh->num_nodes() == 0)
  {
    warning("Input Field does not contain any nodes, setting distance to maximum.");    
    algo_end(); return (true);
  }
      
    
  if (objmesh->num_nodes() == 0)
  {
    warning("Object Field does not contain any nodes, setting distance to maximum.");
    ofield->set_all_values(DBL_MAX);
    
    algo_end(); return (true);
  }
    
  
  objmesh->synchronize(Mesh::FIND_CLOSEST_ELEM_E);

  if (ofield->basis_order() > 2)
  {
    error("Cannot add distance data to field");
    algo_end(); return (false);
  }

  CalculateDistanceFieldP palgo(imesh,objmesh,ofield,this);
  Thread::parallel(&palgo,&CalculateDistanceFieldP::parallel,Thread::numProcessors(),Thread::numProcessors());

  algo_end(); return (true);
}

bool
CalculateDistanceFieldAlgo::
run(FieldHandle input, FieldHandle object, FieldHandle& distance, FieldHandle& value)
{
  algo_start("CalculateDistanceField");
  
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  if (object.get_rep() == 0)
  {
    error("No object field");
    algo_end(); return (false);
  }

  // Determine output type
  FieldInformation fo(input);
  if (fo.is_nodata()) fo.make_lineardata();

  FieldInformation fb(object);

  if (fb.is_nodata())
  {
    error("Object field does not contain any values");
    algo_end(); return (false);  
  }
  // Create Value mesh with same type as object type

  fo.set_data_type(fb.get_data_type());
  value = CreateField(fo,input->mesh());

  if (check_option("basistype","linear")) fo.make_lineardata();
  if (check_option("basistype","constant")) fo.make_constantdata();

  fo.set_data_type(get_option("datatype"));
  distance = CreateField(fo,input->mesh());
  
  if (distance.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return (false);
  }

  if (value.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return (false);
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
    
    algo_end(); return (true);
  }
  
  objmesh->synchronize(Mesh::FIND_CLOSEST_ELEM_E);

  if (distance->basis_order() > 2)
  {
    error("Cannot add distance data to field");
    algo_end(); return (false);
  }

  CalculateDistanceFieldP palgo(imesh,objmesh,objfield,dfield,vfield,this);
  Thread::parallel(&palgo,&CalculateDistanceFieldP::parallel2,Thread::numProcessors(),Thread::numProcessors());

  algo_end(); return (true);

}



} // end namespace SCIRunAlgo
