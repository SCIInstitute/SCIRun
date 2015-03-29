/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Fields/DistanceField/CalculateInsideWhichField.h>

#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
CalculateInsideWhichFieldAlgo::
run(FieldHandle input, std::vector<FieldHandle> objfield, FieldHandle& output)
{
  algo_start("Calculate IsInsideField");
  
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  for (size_t p=0;p<objfield.size();p++)
  {
    if (objfield[p].get_rep() == 0)
    {
      error("No object field");
      algo_end(); return (false);
    }
  }
  
  // no precompiled version available, so compile one

  FieldInformation fi(input);
  FieldInformation fo(input);
    
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    algo_end(); return (false);
  }
    
  std::string output_type = get_option("output_type");  
    
  if (output_type != "same as input")
  {  
    fo.set_data_type(output_type);
  }
 
  
 
  if (fo.is_vector()) fo.make_double();
  if (fo.is_tensor()) fo.make_double();
  
  fo.make_constantdata();
  if (check_option("data_location","node")) fo.make_lineardata();
    
  output = CreateField(fo,input->mesh());
  
  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return(false);  
  }

  // For the moment we calculate everything in doubles

  VField* ifield  = input->vfield();
  
  VMesh*  omesh   = output->vmesh();
  VField* ofield  = output->vfield();

  double outside_value = get_scalar("outside_value");
  double start_value   = get_scalar("start_value");

  bool change_outside_values = get_bool("change_outside_values");
  if (change_outside_values)
    ofield->set_all_values(outside_value);
  else
    ofield->copy_values(ifield);
    
  std::vector<VMesh*> objmesh(objfield.size(),0); 
  
  if (ofield->basis_order() == 0)
  {
    for (size_t p=0;p<objfield.size();p++)
    {
      objmesh[p] = objfield[p]->vmesh();  
      objmesh[p]->synchronize(Mesh::ELEM_LOCATE_E);
    }
    
    VMesh::size_type num_elems = omesh->num_elems();
    
    VMesh::Node::array_type nodes;
    VMesh::Elem::index_type cidx;

    std::vector<Point> points;
    std::vector<Point> points2;
    
    std::vector<VMesh::coords_type> coords;
    std::vector<double> weights;
    
    std::string sampling_scheme = get_option("sampling_scheme");
    if (sampling_scheme == "regular1") omesh->get_regular_scheme(coords,weights,1);
    else if (sampling_scheme == "regular2") omesh->get_regular_scheme(coords,weights,2);
    else if (sampling_scheme == "regular3") omesh->get_regular_scheme(coords,weights,3);
    else if (sampling_scheme == "regular4") omesh->get_regular_scheme(coords,weights,4);
    else if (sampling_scheme == "regular5") omesh->get_regular_scheme(coords,weights,5);

    std::string method = get_option("method");
    
    int cnt = 0;
    
    if (method == "one")
    {
      for(VMesh::Elem::index_type idx=0; idx<num_elems;idx++)
      {
        omesh->minterpolate(points2,coords,idx);

        for (size_t p=0; p<objmesh.size(); p++)
        {
          bool is_inside = false;
          for (size_t r=0; r< points2.size(); r++)
          {
            if (objmesh[p]->locate(cidx,points2[r]))
            {
              is_inside = true; break;
            }
          }

          if (is_inside) ofield->set_value(start_value+p,idx);
        }
        // Progress Reporting
        cnt++; if (cnt == 100) { update_progress(idx,num_elems); cnt = 0; }
      }
    }
    else if (method == "all")
    {
      for(VMesh::Elem::index_type idx=0; idx<num_elems;idx++)
      {
        omesh->minterpolate(points2,coords,idx);

        for (size_t p=0; p<objmesh.size(); p++)
        {
          bool is_inside = true;      
          for (size_t r=0; r< points2.size(); r++)
          {
            if (!(objmesh[p]->locate(cidx,points2[r])))
            {
              is_inside = false; break;
            }
          }
        
          if (is_inside) ofield->set_value(start_value+p,idx);
        }
        // Progress Reporting
        cnt++; if (cnt == 100) { update_progress(idx,num_elems); cnt = 0; }
      }
    }  
    else
    {
      for(VMesh::Elem::index_type idx=0; idx<num_elems;idx++)
      {
        omesh->minterpolate(points2,coords,idx);

        for (size_t p=0; p<objmesh.size(); p++)
        {
          int outside = 0;
          int inside = 0;
          for (size_t r=0; r< points2.size(); r++)
          {
            if (objmesh[p]->locate(cidx,points2[r])) inside++; else outside++;
          }

          if (inside >= outside) ofield->set_value(start_value+p,idx);
        }
        // Progress Reporting
        cnt++; if (cnt == 100) { update_progress(idx,num_elems); cnt = 0; }
      }
    }
  }
  else
  {
    for (size_t p=0;p<objfield.size();p++)
    {
      objmesh[p] = objfield[p]->vmesh();  
      objmesh[p]->synchronize(Mesh::ELEM_LOCATE_E);
    }
    
    VMesh::size_type num_nodes = omesh->num_nodes();
    
    int cnt = 0;
    
    for(VMesh::Node::index_type idx=0; idx<num_nodes;idx++)
    {
      Point point;
      VMesh::Elem::index_type cidx;
      omesh->get_center(point,idx);

      for (size_t p=0; p<objmesh.size(); p++)
      {
        bool is_inside = false;
        if (objmesh[p]->locate(cidx,point))
        {
          ofield->set_value(start_value+p,idx);
        }
      }
        
      // Progress Reporting
      cnt++; if (cnt == 100) { update_progress(idx,num_nodes); cnt = 0; }
    }
  }
  
  algo_end(); return (true);  
}

} // End namespace SCIRunAlgo
