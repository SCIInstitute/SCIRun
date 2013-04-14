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


#include <Core/Algorithms/Fields/Mapping/MapFieldDataFromElemToNode.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

template <class DATA> 
bool 
MapFieldDataFromElemToNodeT(MapFieldDataFromElemToNodeAlgo* algo,
                            FieldHandle& input, 
                            FieldHandle& output);

template <class DATA> 
bool 
MapFieldDataFromElemToNodeT(MapFieldDataFromElemToNodeAlgo* algo,
                            FieldHandle& input, 
                            FieldHandle& output)
{
  std::string method;
  algo->get_option("method",method);
  
  VField *ifield = input->vfield();
  VField *ofield = output->vfield();
    
  //! Make sure that the data vector has the same length
  ofield->resize_fdata();
  
  VMesh* mesh = input->vmesh();

  VMesh::Elem::array_type elems;
  VMesh::Node::iterator it, eit;
  VMesh::Node::size_type sz;

  mesh->synchronize(SCIRun::Mesh::NODE_NEIGHBORS_E);

  mesh->begin(it);
  mesh->end(eit);
  
	mesh->size(sz);
	index_type cnt = 0, c = 0;

  if (method == "interpolate")
  {
    algo->remark("Interpolation of piecewise constant data is done by averaging adjoining values");
  }

  if ((method == "interpolate")||(method == "average"))
  {
    while (it != eit)
    {
      mesh->get_elems(elems, *(it));
      size_t nsize = elems.size();
      DATA val(0);
      DATA tval;
      for (size_t p = 0; p < nsize; p++)
      {
        ifield->get_value(tval,elems[p]);
        val += tval;
      }
      val = static_cast<DATA>(val*(1.0/static_cast<double>(nsize)));
      ofield->set_value(val,*(it));
      ++it;
      cnt++; 
      if (cnt==1000) 
      { 
        cnt=0; c+=1000; 
        algo->update_progress(c,sz); 
        if (algo->check_abort()) break;
      }
    }
  }
  
  if (method=="max")
  {
    while (it != eit)
    {
      mesh->get_elems(elems, *(it));
      size_t nsize = elems.size();
      DATA val(0);
      DATA tval(0);
      if (nsize > 0)
      {
        ifield->get_value(val,elems[0]);
        for (size_t p = 1; p < nsize; p++)
        {
          ifield->get_value(tval,elems[p]);
          if (tval > val) val = tval;
        }
      }
      ofield->set_value(val,*(it));
      ++it;
      cnt++; 
      if (cnt==1000) 
      { 
        cnt=0; c+=1000; 
        algo->update_progress(c,sz); 
        if (algo->check_abort()) break;
      }
    }
  }
  
  if (method=="min")
  {
    while (it != eit)
    {
      mesh->get_elems(elems, *it);
      size_t nsize = elems.size();
      DATA val(0);
      DATA tval(0);
      if (nsize > 0)
      {
        ifield->get_value(val,elems[0]);
        for (size_t p = 1; p < nsize; p++)
        {
          ifield->value(tval,elems[p]);
          if (tval < val) val = tval;
        }
      }
      ofield->set_value(val,*(it));
      ++it;
      cnt++; 
      if (cnt==1000) 
      { 
        cnt=0; c+=1000; 
        algo->update_progress(c,sz); 
        if (algo->check_abort()) break;
      }
    }    
  }

  if (method=="sum")
  {
    while (it != eit)
    {
      mesh->get_elems(elems, *(it));
      size_t nsize = elems.size();
      DATA val(0);
      DATA tval(0);
      for (size_t p = 0; p < nsize; p++)
      {
        ifield->get_value(tval,elems[p]);
        val += tval;
      }
      ofield->set_value(val,*(it));
      ++it;
      cnt++; 
      if (cnt==1000) 
      { 
        cnt=0; c+=1000; 
        algo->update_progress(c,sz); 
        if (algo->check_abort()) break;
      }
    }
  }

  if (method == "median")
  {
    std::vector<DATA> valarray;
    while (it != eit)
    {
      mesh->get_elems(elems, *(it));
      size_t nsize = elems.size();
      valarray.resize(nsize);
      for (size_t p = 0; p < nsize; p++)
      {
        ifield->get_value(valarray[p],elems[p]);
      }
      sort(valarray.begin(),valarray.end());
      int idx = static_cast<int>((valarray.size()/2));
      ofield->set_value(valarray[idx],*(it));
      ++it;
      cnt++; 
      if (cnt==1000) 
      { 
        cnt=0; c+=1000; 
        algo->update_progress(c,sz); 
        if (algo->check_abort()) break;
      }
    }
  }
  
  //! Check whether algorithm was aborted
  if (algo->check_abort())
  {
    //! Data is not valid, hence purge it
    output = 0;
    //! Return an error status to the user
    algo->algo_end(); return (false);
  }
  
  //! Algorithm succeeded
  algo->algo_end(); return (true);
}


bool 
MapFieldDataFromElemToNodeAlgo::
run(FieldHandle& input, FieldHandle& output)
{ 
  algo_start("MapFieldData");
  
  //! Safety check
  if (input.get_rep() == 0)
  {
    error("No input source field");
    algo_end(); return (false);
  }

  //! Get type information
  FieldInformation fi(input);
  FieldInformation fo(input);
  

  //! If input is already on nodes: just passing through
  if (fi.is_lineardata())
  {
    remark("Data is already located at nodes");
    output = input;
    algo_end(); return (true);
  }

  if (!(fi.is_constantdata()))
  {
    error("The input data needs to be located at the elements");
    algo_end(); return (false);  
  }

  //! Select output type
  fo.make_lineardata();

  //! Create output field
  output = CreateField(fo,input->mesh());

  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return(false);
  }
  
  
  if (input->vfield()->is_signed_integer()) 
    return (MapFieldDataFromElemToNodeT<int>(this,input,output));

  if (input->vfield()->is_unsigned_integer()) 
    return (MapFieldDataFromElemToNodeT<unsigned int>(this,input,output));

  if (input->vfield()->is_scalar()) 
    return (MapFieldDataFromElemToNodeT<double>(this,input,output));
  
 if (input->vfield()->is_vector()) 
    return (MapFieldDataFromElemToNodeT<Vector>(this,input,output));
 
  if (input->vfield()->is_tensor()) 
    return (MapFieldDataFromElemToNodeT<Tensor>(this,input,output));

  error("Encountered an unknown data type");

  algo_end(); return (false);
}

} // namespace SCIRunAlgo
