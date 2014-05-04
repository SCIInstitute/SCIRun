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

#include <Core/Algorithms/Fields/Mapping/MapFieldDataFromNodeToElem.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

/// Internal function to this algorithm: no need for this function to be
/// public. It is called from the algorithm class only.

template <class DATA> 
bool 
MapFieldDataFromNodeToElemT(MapFieldDataFromNodeToElemAlgo* algo,
                            FieldHandle& input, 
                            FieldHandle& output);

/// This is the basic algorithm behind the mapping algorithm

template <class DATA> 
bool 
MapFieldDataFromNodeToElemT(MapFieldDataFromNodeToElemAlgo* algo,
                            FieldHandle& input, 
                            FieldHandle& output)
{
  /// Get the method the user selected.
  /// Since we do a check of valid entries when then user sets the
  /// algorithm, we can assume it is one of the specified ones
  std::string method;
  algo->get_option("method",method);

  /// Get pointers to the virtual interfaces of the fields
  /// We need these to obtain the data values
  
  VField *ifield = input->vfield();
  VField *ofield = output->vfield();
  
  /// Make sure that the data vector has the proper length  
  VMesh* mesh = input->vmesh();

  VMesh::Elem::array_type elems;
  VMesh::Elem::iterator it, eit;
  VMesh::Elem::size_type sz;

  VMesh::Node::array_type nodearray;

  mesh->begin(it);
  mesh->end(eit);
  mesh->size(sz);
  index_type cnt = 0, c = 0;

  if ((method == "average") || (method == "interpolate"))
  {
    DATA tval(0);
    
    while (it != eit)
    {
      mesh->get_nodes(nodearray, *it);
      size_t nsize = nodearray.size();
      DATA val(0);
      for (size_t p = 0; p < nsize; p++)
      {
        ifield->get_value(tval,nodearray[p]);
        val += tval;
      }
      val = static_cast<DATA>(val * static_cast<double>((1.0/static_cast<double>(nsize))));
      ofield->set_value(val,*it);
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
  else if (method == "max")
  {
    while (it != eit)
    {
      mesh->get_nodes(nodearray, *it);
      size_t nsize = nodearray.size();
      DATA val(0);
      DATA tval(0);
      if (nsize > 0)
      {
        ifield->get_value(val,nodearray[0]);
        for (size_t p = 1; p < nsize; p++)
        {
          ifield->get_value(tval,nodearray[p]);
          if (val < tval) val = tval;
        }
      }
      ofield->set_value(val,*it);
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
  else if (method == "min")
  {
    DATA tval(0);
    while (it != eit)
    {
      mesh->get_nodes(nodearray, *it);
      size_t nsize = nodearray.size();
      DATA val(0);
      if (nsize > 0)
      {
        ifield->get_value(val,nodearray[0]);
        for (size_t p = 1; p < nsize; p++)
        {
          ifield->value(tval,nodearray[p]);
          if (tval < val) val = tval;
        }
      }
      ofield->set_value(val,*it);
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
  else if (method == "sum")
  {
    DATA tval(0);
    while (it != eit)
    {
      mesh->get_nodes(nodearray, *it);
      size_t nsize = nodearray.size();
      DATA val(0);
      for (size_t p = 0; p < nsize; p++)
      {
        ifield->get_value(tval,nodearray[p]);
        val += tval;
      }
      ofield->set_value(val,*it);
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
  else if (method == "median")
  {
    std::vector<DATA> valarray;
    while (it != eit)
    {
      mesh->get_nodes(nodearray, *it);
      size_t nsize = nodearray.size();
      valarray.resize(nsize);
      for (size_t p = 0; p < nsize; p++)
      {
        ifield->get_value(valarray[p],nodearray[p]);
      }
      sort(valarray.begin(),valarray.end());
      int idx = static_cast<int>((valarray.size()/2));
      ofield->set_value(valarray[idx],*it);        
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
  
  /// Check whether algorithm was aborted
  if (algo->check_abort())
  {
    /// Data is not valid, hence purge it
    output = 0;
    /// Return an error status to the user
    algo->algo_end(); return (false);
  }
  
  /// Algorithm succeeded
  algo->algo_end(); return (true);
}


/// Actual Algorithm class

bool 
MapFieldDataFromNodeToElemAlgo::
run(FieldHandle& input, FieldHandle& output)
{
  algo_start("MapFieldData");
  
  /// safety check
  if (input.get_rep() == 0)
  {
    error("No input source field");
    algo_end(); return (false);
  }

  /// Get information about field types
  FieldInformation fi(input);
  FieldInformation fo(input);
  
  /// In case data is already on the elements
  if (fi.is_constantdata())
  {
    remark("Data is already at elements");
    output = input;
    algo_end(); return (true);
  }

  /// We need linear data to start with
  if (!(fi.is_lineardata()))
  {
    error("This function needs to have data at the nodes");
    algo_end(); return (false);  
  }

  /// Make sure output type has data on elements
  fo.make_constantdata();
  
  /// Create output field
  output = CreateField(fo,input->mesh());
 
  /// Check whether output field was created
  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return(false);
  } 
  
  /// Simple table to deal with the various data type formats
  /// Note that not every data type is handled, all char, shorts etc,
  /// are automatically handled by the int, and unsigned int case, by
  /// casting the data on input (these should be the less frequently
  /// used datatypes and hence have no specific algorithm in place).
  /// Similarly floats are casted to doubles.
  
  if (input->vfield()->is_signed_integer()) 
    return (MapFieldDataFromNodeToElemT<int>(this,input,output));

  if (input->vfield()->is_unsigned_integer()) 
    return (MapFieldDataFromNodeToElemT<unsigned int>(this,input,output));

  if (input->vfield()->is_scalar()) 
    return (MapFieldDataFromNodeToElemT<double>(this,input,output));
  
 if (input->vfield()->is_vector()) 
    return (MapFieldDataFromNodeToElemT<Vector>(this,input,output));
 
  if (input->vfield()->is_tensor()) 
    return (MapFieldDataFromNodeToElemT<Tensor>(this,input,output));

  error("Encountered an unknown data type");
  algo_end(); return (false);
}

} // namespace SCIRunAlgo
