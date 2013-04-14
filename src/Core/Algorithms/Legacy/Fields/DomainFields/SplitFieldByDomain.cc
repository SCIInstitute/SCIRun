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

#include <Core/Algorithms/Fields/DomainFields/SplitFieldByDomain.h>

#include <Core/Datatypes/FieldInformation.h>
#include <algorithm>

namespace SCIRunAlgo {

using namespace SCIRun;

class SortSizes : public std::binary_function<index_type,index_type,bool>
{
  public:
    SortSizes(double* sizes) : sizes_(sizes) {}
    
    bool operator()(index_type i1, index_type i2)
    {
      return (sizes_[i1] > sizes_[i2]);
    }

  private:
    double*      sizes_;
};


class AscSortSizes : public std::binary_function<index_type,index_type,bool>
{
  public:
    AscSortSizes(double* sizes) : sizes_(sizes) {}
    
    bool operator()(index_type i1, index_type i2)
    {
      return (sizes_[i1] < sizes_[i2]);
    }

  private:
    double*      sizes_;
};



bool 
SplitFieldByDomainAlgo::
run( FieldHandle input, std::vector<FieldHandle>& output)
{
  algo_start("SplitNodesByDomain");
  
  if (input.get_rep() == 0)
  {
    algo_end(); error("No input field");
    return (false);
  }

  FieldInformation fi(input), fo(input); 
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements.");
    algo_end(); return (false);
  }

  if (!(fi.is_constantdata()))
  {
    error("This function only works for data located at the elements");
    algo_end(); return (false);
  }

  fo.make_unstructuredmesh();
  fo.make_int();

  VField *field = input->vfield();
  VMesh  *mesh  = input->vmesh();
  
  if (field == 0 || mesh == 0)
  {
    algo_end(); error("No input field");
    return (false);
  }

  VMesh::Elem::iterator bei, eei;
  VMesh::Node::iterator bni, eni;
  
  VMesh::size_type num_elems = mesh->num_elems();
  VMesh::size_type num_nodes = mesh->num_nodes();


  std::vector<VMesh::Node::index_type> idxarray(num_nodes);
  std::vector<bool> newidxarray(num_nodes);
  VMesh::Node::array_type nodes;
  VMesh::Node::array_type newnodes;
  VMesh::Node::index_type node;
    
  std::vector<int> labels;
  field->get_values(labels);
  
  int flabel = 0;
  if (labels.size()) flabel = labels[0];
  for (size_t j=0; j<labels.size();j++) { if (labels[j] < flabel) flabel = labels[j]; }

  index_type k = 0;
  
  int cnt = 0;
  while(1)
  {
    FieldHandle output_field = CreateField(fo);
    
    if (output_field.get_rep() == 0)
    {
      error("Could not create output field");
      output.clear();
      algo_end(); return(false); 
    }
   
    VField* ofield = output_field->vfield();
    VMesh *omesh = output_field->vmesh();

    if (ofield == 0 || omesh == 0)
    {
      error("Could not create output field");
      output.clear();
      algo_end(); return(false); 
    }
    
    for (size_type p =0; p<num_nodes; p++) newidxarray[p] = true;

    for (VMesh::Elem::index_type idx=0; idx< num_elems; idx++)
    {
      if (labels[idx] == flabel)
      {
        mesh->get_nodes(nodes,idx);
        newnodes.resize(nodes.size());
        for (size_t p=0; p< nodes.size(); p++)
        {
          node = nodes[p];
          if (newidxarray[node])
          {
            Point pt;
            mesh->get_center(pt,nodes[p]);
            idxarray[node] = omesh->add_point(pt);
            newidxarray[node] = false;
          }
          newnodes[p] = idxarray[node];
        }
        omesh->add_elem(newnodes);
        cnt++; if (cnt == 400) { cnt = 0; update_progress(k,num_elems); }   
        k++;
      }
    }

    ofield->resize_values();
    ofield->set_all_values(flabel);
    output.push_back(output_field);

    int elabel = 0;
    bool found = false;
    
    for (VMesh::index_type lidx = 0; lidx < num_elems; lidx++)
    {
      if (labels[lidx] > flabel)
      {
        if (!found) { elabel = labels[lidx]; found = true;}
        if (labels[lidx] < elabel) elabel = labels[lidx]; 
      }
    }
    // check whether we are done
    if (!found) break;
    flabel = elabel;    
  }
  
  
  if (get_bool("sort_by_size"))
  {
    std::vector<double> sizes(output.size());
    std::vector<index_type> order(output.size());
    std::vector<FieldHandle> temp(output.size());
    
    for (size_t j=0; j<output.size(); j++)
    {
      VMesh* mesh = output[j]->vmesh();
      VMesh::Elem::size_type num_elems = mesh->num_elems();
      double size = 0.0;
      for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
      {
        size += mesh->get_size(idx);
      }
      sizes[j] = size;
      order[j] = j;
      temp[j] = output[j];
    }
  
    if (get_bool("sort_ascending"))
    {
      std::sort(order.begin(),order.end(),AscSortSizes(&(sizes[0]))); 
    }
    else
    {
      std::sort(order.begin(),order.end(),SortSizes(&(sizes[0]))); 
    }
    for (size_t j=0; j<output.size(); j++)
    {
      output[j] = temp[order[j]];
    }
  }
  
  algo_end(); return(true);
}

} // namespace SCIRunAlgo

