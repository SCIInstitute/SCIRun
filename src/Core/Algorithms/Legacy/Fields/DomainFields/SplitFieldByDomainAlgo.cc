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


#include <Core/Algorithms/Legacy/Fields/DomainFields/SplitFieldByDomainAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <algorithm>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

AlgorithmParameterName SplitFieldByDomainAlgo::SortAscending("SortAscending");
AlgorithmParameterName SplitFieldByDomainAlgo::SortBySize("SortBySize");

SplitFieldByDomainAlgo::SplitFieldByDomainAlgo()
{
  addParameter(SortBySize, false);
  addParameter(SortAscending, false);
}

namespace {

class SortSizes : public std::binary_function<index_type,index_type,bool>
{
  public:
    explicit SortSizes(const std::vector<double>& sizes) : sizes_(sizes) {}

    bool operator()(index_type i1, index_type i2) const
    {
      return (sizes_[i1] > sizes_[i2]);
    }

  private:
    const std::vector<double>& sizes_;
};


class AscSortSizes : public std::binary_function<index_type,index_type,bool>
{
  public:
    explicit AscSortSizes(const std::vector<double>& sizes) : sizes_(sizes) {}

    bool operator()(index_type i1, index_type i2) const
    {
      return (sizes_[i1] < sizes_[i2]);
    }

  private:
    const std::vector<double>& sizes_;
};
}


bool
SplitFieldByDomainAlgo::runImpl(FieldHandle input, FieldList& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "SplitNodesByDomain");

  if (!input)
  {
    error("No input field");
    return (false);
  }

  FieldInformation fi(input), fo(input);
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements.");
    return (false);
  }

  if (!(fi.is_constantdata()))
  {
    error("This function only works for data located at the elements");
    return (false);
  }

  fo.make_unstructuredmesh();
  fo.make_int();

  VField *field = input->vfield();
  VMesh  *mesh  = input->vmesh();

  if (!field || !mesh)
  {
    error("No input field");
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
  while (true)
  {
    FieldHandle output_field = CreateField(fo);

    if (!output_field)
    {
      error("Could not create output field");
      output.clear();
      return(false);
    }

    VField* ofield = output_field->vfield();
    VMesh *omesh = output_field->vmesh();

    if (!ofield || !omesh)
    {
      error("Could not create output field");
      output.clear();
      return(false);
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
        cnt++;
        if (cnt == 400)
        {
          cnt = 0;
          update_progress_max(k,num_elems);
        }
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


  if (get(SortBySize).toBool())
  {
    std::vector<double> sizes(output.size());
    std::vector<index_type> order(output.size());
    std::vector<FieldHandle> temp(output);

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
    }

    if (get(SortAscending).toBool())
    {
      std::sort(order.begin(),order.end(),AscSortSizes(sizes));
    }
    else
    {
      std::sort(order.begin(),order.end(),SortSizes(sizes));
    }
    // easier just to sort output directly?
    for (size_t j=0; j<output.size(); j++)
    {
      output[j] = temp[order[j]];
    }
  }

  std::ostringstream ostr;
  ostr << "Input field split into " << output.size() << " fields.";
  remark(ostr.str());

  return(true);
}

AlgorithmOutput SplitFieldByDomainAlgo::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);

  FieldList list;
  if (!runImpl(field, list))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output.setList(Variables::ListOfOutputFields, list);
  return output;
}
