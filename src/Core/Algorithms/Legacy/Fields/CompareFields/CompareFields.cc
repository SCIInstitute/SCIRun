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


#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>

#include <Core/Algorithms/Fields/CompareFields/CompareFields.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool CompareFieldsAlgo::compare(FieldHandle fieldh1, FieldHandle fieldh2)
{
  algo_start("CompareFields");

  if (fieldh1.get_rep() == fieldh2.get_rep())
  {
    algo_end(); return (true);
  }

  if (fieldh1.get_rep() == 0)
  {
    remark("Field 1 is empty");
    algo_end(); return (false);
  }

  if (fieldh2.get_rep() == 0)
  {
    remark("Field 2 is empty");
    algo_end(); return (false);
  }

  // no precompiled version available, so compile one

  FieldInformation fi1(fieldh1);
  FieldInformation fi2(fieldh2);

  if (!(fi1 == fi2))
  {
    remark("CompareFields: Field types are not equal");
    algo_end(); return (false);
  }

  VMesh* imesh1 = fieldh1->vmesh();
  VMesh* imesh2 = fieldh2->vmesh();
  VField* ifield1 = fieldh1->vfield();
  VField* ifield2 = fieldh2->vfield();

  VMesh::Node::size_type numnodes1, numnodes2;
  VMesh::Elem::size_type numelems1, numelems2;
  imesh1->size(numnodes1);
  imesh1->size(numelems1);
  imesh2->size(numnodes2);
  imesh2->size(numelems2);

  if (numnodes1 != numnodes2)
  {
    remark("Number of nodes is not equal");
    algo_end(); return (false);
  }

  if (numelems1 != numelems2)
  {
    remark("Number of elements is not equal");
    algo_end(); return (false);
  }

  VMesh::Node::iterator nbi1, nei1;
  VMesh::Node::iterator nbi2, nei2;

  imesh1->begin(nbi1);
  imesh2->begin(nbi2);
  imesh1->end(nei1);
  imesh2->end(nei2);

  while (nbi1 != nei1)
  {
    Point p1, p2;
    imesh1->get_center(p1,*nbi1);
    imesh2->get_center(p2,*nbi2);

    if (p1 != p2)
    {
      remark("The nodes are not equal");
      algo_end(); return (false);
    }
    ++nbi1;
    ++nbi2;
  }

  VMesh::Elem::iterator ebi1, eei1;
  VMesh::Elem::iterator ebi2, eei2;
  VMesh::Node::array_type nodes1, nodes2;

  imesh1->begin(ebi1);
  imesh2->begin(ebi2);
  imesh1->end(eei1);
  imesh2->end(eei2);

  while (ebi1 != eei1)
  {
    Point p1, p2;
    imesh1->get_nodes(nodes1,*ebi1);
    imesh2->get_nodes(nodes2,*ebi2);

    if (nodes1.size() != nodes2.size())
    {
      remark("The number of nodes per element are not equal");
      algo_end(); return (false);
    }

    for (size_t p=0; p<nodes1.size(); p++)
    {
      if (nodes1[p] != nodes2[p])
      {
        remark("The nodes that define the element are not equal");
        algo_end(); return (false);
      }
    }
    ++ebi1;
    ++ebi2;
  }

  // TO DO:
  // SHould make this code more efficient by adding a compare function
  // to VField

  if (ifield1->basis_order() == 0)
  {
    imesh1->begin(ebi1);
    imesh1->end(eei1);
    imesh2->begin(ebi2);
    imesh2->end(eei2);

    double val1, val2;

    while (ebi1 != eei1)
    {

      ifield1->value(val1,*ebi1);
      ifield2->value(val2,*ebi2);

      if (val1 != val2)
      {
        remark("The values in the fields are not equal");
        algo_end(); return (false);
      }
      ++ebi1;
      ++ebi2;
    }
  }

  if (ifield1->basis_order() == 1)
  {
    imesh1->begin(nbi1);
    imesh1->end(nei1);
    imesh2->begin(nbi2);
    imesh2->end(nei2);

    double val1, val2;

    while (nbi1 != nei1)
    {

      ifield1->value(val1,*nbi1);
      ifield2->value(val2,*nbi2);

      if (val1 != val2)
      {
        remark("The values in the fields are not equal");
        algo_end(); return (false);
      }
      ++nbi1;
      ++nbi2;
    }
  }

  algo_end(); return (true);
}


} // End namespace SCIRunAlgo
