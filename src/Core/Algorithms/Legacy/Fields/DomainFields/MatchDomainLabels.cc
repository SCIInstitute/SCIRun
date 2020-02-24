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


#include <Core/Algorithms/Fields/DomainFields/MatchDomainLabels.h>

#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
MatchDomainLabelsAlgo::
run( FieldHandle input, FieldHandle domain, FieldHandle& output)
{
  algo_start("MatchDomainLabels");

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

  fo.make_int();

  VField *ifield = input->vfield();
  VMesh  *imesh  = input->vmesh();

  VField *dfield = domain->vfield();
  VMesh  *dmesh  = domain->vmesh();


  if (ifield == 0 || imesh == 0)
  {
    algo_end(); error("No input field");
    return (false);
  }

  output = CreateField(fo,input->mesh());

  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return(false);
  }

  VField* ofield = output->vfield();
  VMesh *omesh = output->vmesh();

  if (ofield == 0 || omesh == 0)
  {
    error("Could not create output field");
    algo_end(); return(false);
  }

  ofield->resize_values();
  int* olabels = reinterpret_cast<int*>(ofield->get_values_pointer());

  std::vector<int> labels;
  ifield->get_values(labels);
  VMesh::size_type num_elems = imesh->num_elems();

  int flabel = 0;
  if (labels.size() > 0) flabel = labels[0];
  // Find minimum
  for (size_t j=0; j<labels.size();j++) if (flabel > labels[j]) flabel = labels[j];

  int dlabel;
  std::vector<std::pair<int,double> > histogram;
  Point p;

  dmesh->synchronize(Mesh::ELEM_LOCATE_E);

  std::vector<double> sizes(num_elems);
  for (VMesh::Elem::index_type idx = 0; idx< num_elems; idx++)
    sizes[idx] = imesh->get_size(idx);

  while(1)
  {

    for (VMesh::Elem::index_type idx = 0; idx< num_elems; idx++)
    {
      if (labels[idx] == flabel)
      {
        imesh->get_center(p,idx);
        if(dfield->interpolate(dlabel,p))
        {
          size_t k = 0;
          for(; k<histogram.size();k++)
          {
            if(histogram[k].first == dlabel)
            {
              histogram[k].second += sizes[idx];
              break;
            }
          }
          if (k == histogram.size()) histogram.push_back(std::pair<int,double>(dlabel,sizes[idx]));
        }
      }
    }


    int newlabel = 0;
    double count = 0;
    if (histogram.size() > 0)
    {
      newlabel = histogram[0].first;
      count = histogram[0].second;
    }

    for (size_t j=0; j<histogram.size(); j++)
    {
      if (histogram[j].second > count) newlabel = histogram[j].first;
    }

    for (VMesh::index_type idx = 0; idx< num_elems; idx++)
    {
      if (labels[idx] == flabel)
      {
        olabels[idx] = newlabel;
      }
    }

    histogram.clear();

    int elabel = 0;
    bool found = false;

    for (VMesh::index_type lidx=0; lidx < num_elems; lidx++)
    {
      if (labels[lidx] > flabel)
      {
        if (!found)
        {
          elabel = labels[lidx];
          found = true;
        }
        if (labels[lidx] < elabel) elabel = labels[lidx];
      }
    }
    // check whether we are done
    if (!found) break;
    flabel = elabel;
  }

  algo_end();
  return(true);
}

} // namespace SCIRunAlgo
