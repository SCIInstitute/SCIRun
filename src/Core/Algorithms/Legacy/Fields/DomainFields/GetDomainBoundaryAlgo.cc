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


#include <Core/Algorithms/Legacy/Fields/DomainFields/GetDomainBoundaryAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/PropertyManagerExtensions.h>
#include <Core/Logging/Log.h>

#include <boost/unordered_map.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;

ALGORITHM_PARAMETER_DEF(Fields, MinRange);
ALGORITHM_PARAMETER_DEF(Fields, MaxRange);
ALGORITHM_PARAMETER_DEF(Fields, Domain);
ALGORITHM_PARAMETER_DEF(Fields, UseRange);
ALGORITHM_PARAMETER_DEF(Fields, AddOuterBoundary);
ALGORITHM_PARAMETER_DEF(Fields, InnerBoundaryOnly);
ALGORITHM_PARAMETER_DEF(Fields, NoInnerBoundary);
ALGORITHM_PARAMETER_DEF(Fields, DisconnectBoundaries);

struct pointtype
{
  pointtype() : node(0), val1(0), val2(0), hasneighbor(false) {}
  VMesh::Node::index_type node;
  int val1;
  int val2;
  bool hasneighbor;
};

AlgorithmInputName GetDomainBoundaryAlgo::ElemLink("ElemLink");
AlgorithmOutputName GetDomainBoundaryAlgo::BoundaryField("BoundaryField");

GetDomainBoundaryAlgo::GetDomainBoundaryAlgo()
{
  using namespace Parameters;
  addParameter(MinRange, 0);
  addParameter(MaxRange,255);
  addParameter(Domain,1);
  addParameter(UseRange,false);
  addParameter(AddOuterBoundary,true);
  addParameter(InnerBoundaryOnly,false);
  addParameter(NoInnerBoundary,false);
  addParameter(DisconnectBoundaries,false);
}

bool
GetDomainBoundaryAlgo::runImpl(FieldHandle input, SparseRowMatrixHandle domainlink, FieldHandle& output) const
{
  typedef boost::unordered_multimap<index_type,pointtype> pointhash_map_type;

  ScopedAlgorithmStatusReporter asr(this, "GetDomainBoundary");
  using namespace Parameters;

  int minval = get(MinRange).toInt();
  int maxval = get(MaxRange).toInt();
  const int domval = get(Domain).toInt();

  const bool userange = get(UseRange).toBool();

  if (!userange)
  {
    minval = domval;
    maxval = domval;
  }

  bool addouterboundary = get(AddOuterBoundary).toBool();
  bool innerboundaryonly = get(InnerBoundaryOnly).toBool();
  bool noinnerboundary = get(NoInnerBoundary).toBool();
  bool disconnect = get(DisconnectBoundaries).toBool();

  LOG_DEBUG("GetDomainBoundaryAlgo parameters:\n\tminval = {}"
    "\n\tmaxval = {}\n\tdomval = {}\n\tuserange = {}\n\taddouterboundary = {}"
    "\n\tinnerboundaryonly = {}\n\tnoinnerboundary = {}\n\tdisconnect = {}",
    minval, maxval, domval, userange, addouterboundary, innerboundaryonly,
    noinnerboundary, disconnect);

  if (!input)
  {
    error("No input field");
    return false;
  }

  FieldInformation fi(input);
  FieldInformation fo(input);

  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    return false;
  }

  if (!(fi.is_constantdata()))
  {
    error("This function needs a compartment definition on the elements (constant element data)");
    return false;
  }

  if (!(fi.is_volume()||fi.is_surface()))
  {
    error("This function is only defined for surface and volume data");
    return false;
  }

  if (fi.is_hex_element())
  {
    fo.make_quadsurfmesh();
  }
  else if (fi.is_quad_element()||fi.is_tri_element())
  {
    fo.make_curvemesh();
  }
  else if (fi.is_tet_element())
  {
    fo.make_trisurfmesh();
  }
  else if (fi.is_crv_element())
  {
    fo.make_pointcloudmesh();
  }
  else
  {
    error("No method available for input mesh");
    return false;
  }

  fo.make_constantdata();
  fo.make_int();
  output = CreateField(fo);


  if (!output)
  {
    error("Could not create output field");
    return false;
  }

  auto ifield = input->vfield();
  auto ofield = output->vfield();
  auto imesh =  input->vmesh();
  auto omesh =  output->vmesh();

  imesh->synchronize(Mesh::DELEMS_E|Mesh::ELEM_NEIGHBORS_E|Mesh::NODE_NEIGHBORS_E);

  VMesh::DElem::size_type numdelems = imesh->num_delems();

  bool isdomlink = false;
  const index_type* domlinkrr = nullptr;
  const index_type* domlinkcc = nullptr;

  std::vector<int> newvalues;

  if (domainlink)
  {
    if ((numdelems != domainlink->nrows())&&(numdelems != domainlink->ncols()))
    {
      error("The Domain Link property is not of the right dimensions");
      return false;
    }
    domlinkrr = domainlink->get_rows();
    domlinkcc = domainlink->get_cols();
    isdomlink = true;
  }

  if (disconnect)
  {
    pointhash_map_type node_map;

    VMesh::Elem::index_type nci, ci;
    VMesh::Elem::array_type elements;
    VMesh::DElem::array_type delems;
    VMesh::Node::array_type inodes;
    VMesh::Node::array_type onodes;
    VMesh::Node::index_type a;

    int val1, val2, newval;

    Point point;

    index_type cnt = 0;

    for(VMesh::DElem::index_type delem = 0; delem < numdelems; ++delem)
    {
      checkForInterruption();

      bool neighborexist = false;
      bool includeface = false;

      imesh->get_elems(elements,delem);
      ci = elements[0];
      if (elements.size() > 1)
      {
        neighborexist = true;
        nci  = elements[1];
      }

      if ((!neighborexist)&&(isdomlink))
      {
        for (auto rr = domlinkrr[delem]; rr < domlinkrr[delem+1]; rr++)
        {
          VMesh::DElem::index_type idx = domlinkcc[rr];
          VMesh::Node::array_type nodes;
          VMesh::Elem::array_type elems;
          VMesh::DElem::array_type delems2;

          imesh->get_nodes(nodes,idx);
          imesh->get_elems(elems,nodes[0]);

          for (size_t r=0; r<elems.size(); r++)
          {
            imesh->get_delems(delems2,elems[r]);

            for (size_t s=0; s<delems2.size(); s++)
            {
              if (delems2[s]==idx)
              {
                nci = elems[r];
                neighborexist = true;
                break;
              }
            }
            if (neighborexist) break;
          }
          if (neighborexist) break;
        }
      }

      if (neighborexist)
      {
        ifield->value(val1,ci);
        ifield->value(val2,nci);
        if (!innerboundaryonly)
        {
          if (noinnerboundary)
          {
            if ((((val1 >= minval)&&(val1 <= maxval)&&
                (!((val2 >= minval)&&(val2 <= maxval))))&&
                (userange)))
            {
              newval = val1;
              includeface = true;
            }
            else if (((val2 >= minval)&&(val2 <= maxval)&&
                (!((val1 >= minval)&&(val1 <= maxval))))&&
                (userange))
            {
              newval = val2;
              includeface = true;
            }
          }
          else
          {
            if ((((val1 >= minval)&&(val1 <= maxval))||
                ((val2 >= minval)&&(val2 <= maxval)))||
                (!userange))
            {
              if (!(val1 == val2))
              {
                includeface = true;
                if((val1 >= minval)&&(val1 <= maxval)) newval = val1;
                else newval = val2;
              }
            }
          }
        }
        else
        {
          if ((((val1 >= minval)&&(val2 >= minval))&&
              ((val1 <= maxval)&&(val2 <= maxval)))||
              (!userange))
          {
            if (!(val1 == val2))
            {
              includeface = true;
              newval = val1;
              if (val2 < newval) newval = val2;
            }
          }
        }
      }
      else if ((addouterboundary)&&(!innerboundaryonly))
      {
        ifield->value(val1,ci);
        if (((val1 >= minval)&&(val1 <= maxval))||(!userange))
        {
          newval = val1;
          includeface = true;
        }
      }

      if (includeface)
      {
        imesh->get_nodes(inodes,delem);
        onodes.resize(inodes.size());
        for (size_t q=0; q< onodes.size(); q++)
        {
          checkForInterruption();
          a = inodes[q];

          std::pair<pointhash_map_type::iterator,pointhash_map_type::iterator> lit;
          lit = node_map.equal_range(a);

          VMesh::Node::index_type nodci;
          int v1, v2;
          bool hasneighbor;

          if (neighborexist)
          {
            if (val1 < val2) { v1 = val1; v2 = val2; } else { v1 = val2; v2 = val1; }
            hasneighbor = true;
          }
          else
          {
            v1 = val1; v2 = 0;
            hasneighbor = false;
          }

          while (lit.first != lit.second)
          {
            if (((*(lit.first)).second.val1 == v1)&&
                ((*(lit.first)).second.val2 == v2)&&
                ((*(lit.first)).second.hasneighbor == hasneighbor))
            {
              nodci = (*(lit.first)).second.node;
              break;
            }
            ++(lit.first);
          }

          if (lit.first == lit.second)
          {
            pointtype newpoint;
            imesh->get_center(point,a);
            onodes[q] = omesh->add_point(point);
            newpoint.node = onodes[q];
            newpoint.val1 = v1;
            newpoint.val2 = v2;
            newpoint.hasneighbor = hasneighbor;
            node_map.insert(pointhash_map_type::value_type(a,newpoint));
          }
          else
          {
            onodes[q] = nodci;
          }

        }
        omesh->add_elem(onodes);
        newvalues.push_back(newval);
      }
      cnt++; if (cnt == 100) update_progress_max(delem,numdelems);
    }
  }
  else
  {
    std::vector<VMesh::Node::index_type> node_map(imesh->num_nodes(),-1);

    VMesh::Elem::index_type nci, ci;
    VMesh::Elem::array_type elements;
    VMesh::DElem::array_type delems;
    VMesh::Node::array_type inodes;
    VMesh::Node::array_type onodes;
    VMesh::Node::index_type a;
    int val1, val2, newval;

    Point point;

    index_type cnt = 0;

    for(VMesh::DElem::index_type delem = 0; delem < numdelems; ++delem)
    {
      checkForInterruption();

      bool neighborexist = false;
      bool includeface = false;

      imesh->get_elems(elements,delem);
      ci = elements[0];
      if (elements.size() > 1)
      {
        neighborexist = true;
        nci  = elements[1];
      }

      if ((!neighborexist)&&(isdomlink))
      {
        for (auto rr = domlinkrr[delem]; rr < domlinkrr[delem+1]; rr++)
        {
          VMesh::DElem::index_type idx = domlinkcc[rr];
          VMesh::Node::array_type nodes;
          VMesh::Elem::array_type elems;
          VMesh::DElem::array_type delems2;

          imesh->get_nodes(nodes,idx);
          imesh->get_elems(elems,nodes[0]);

          for (size_t r=0; r<elems.size(); r++)
          {
            imesh->get_delems(delems2,elems[r]);

            for (size_t s=0; s<delems2.size(); s++)
            {
              if (delems2[s]==idx) { nci = elems[r]; neighborexist = true; break; }
            }
            if (neighborexist) break;
          }
          if (neighborexist) break;
        }
      }

      if (neighborexist)
      {
        ifield->value(val1,ci);
        ifield->value(val2,nci);
        if (!innerboundaryonly)
        {
          if (noinnerboundary)
          {
            if (((val1 >= minval)&&(val1 <= maxval)&&
                (!((val2 >= minval)&&(val2 <= maxval))))&&
                (userange))
            {
              newval = val1;
              includeface = true;
            }
            else if(((val2 >= minval)&&(val2 <= maxval)&&
                (!((val1 >= minval)&&(val1 <= maxval))))&&
                (userange))
            {
              newval = val2;
              includeface = true;
            }
          }
          else
          {
            if ((((val1 >= minval)&&(val1 <= maxval))||
                ((val2 >= minval)&&(val2 <= maxval)))||
                (!userange))
            {
              if (!(val1 == val2))
              {
                includeface = true;
                if((val1 >= minval)&&(val1 <= maxval)) newval = val1;
                else newval = val2;
              }
            }
          }
        }
        else
        {
          if ((((val1 >= minval)&&(val2 >= minval))&&
              ((val1 <= maxval)&&(val2 <= maxval)))||
              (!userange))
          {
            if (!(val1 == val2))
            {
              includeface = true;
              newval = val1;
              if (val2 < newval) newval = val2;
            }
          }
        }
      }
      else if ((addouterboundary)&&(!innerboundaryonly))
      {
        ifield->value(val1,ci);
        if (((val1 >= minval)&&(val1 <= maxval))||
            (!userange))
        {
          newval = val1;
          includeface = true;
        }
      }

      if (includeface)
      {
        imesh->get_nodes(inodes,delem);
        onodes.resize(inodes.size());

        for (size_t q=0; q< onodes.size(); q++)
        {
          checkForInterruption();
          a = inodes[q];
          if (node_map[a] == -1)
          {
            imesh->get_center(point,a);
            onodes[q] = omesh->add_point(point);
            node_map[a] = onodes[q];
          }
          else
          {
            onodes[q] = node_map[a];
          }

        }
        omesh->add_elem(onodes);
        newvalues.push_back(newval);
      }
      cnt++; if (cnt == 100) update_progress_max(delem,numdelems);
    }
  }

  ofield->resize_values();
  ofield->set_values(newvalues);

  CopyProperties(*input, *output);
  return true;
}

AlgorithmOutput GetDomainBoundaryAlgo::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);
  auto elemlink = input.get<SparseRowMatrix>(ElemLink);

  FieldHandle boundary;
  if (!runImpl(field, elemlink, boundary))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[BoundaryField] = boundary;
  return output;
}
