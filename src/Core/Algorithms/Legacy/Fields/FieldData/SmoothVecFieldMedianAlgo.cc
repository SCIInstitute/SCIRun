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


#include <Core/Algorithms/Legacy/Fields/FieldData/SmoothVecFieldMedianAlgo.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
//#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Math/MiscMath.h>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

bool SmoothVecFieldMedianAlgo::runImpl(FieldHandle input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "SmoothVecFieldMedian");

  if (!input)
  {
    error("No input field");
    return false;
  }

  FieldInformation fi(input);

  if (fi.is_nodata())
  {
    error("Input field does not have data associated with it");
    return false;
  }

  if (!fi.is_vector())
  {
    error("The data needs to be of vector type");
    return false;
  }

  if (!fi.is_constantdata())
  {
    THROW_ALGORITHM_PROCESSING_ERROR("The data needs to be on the cells of the mesh");
  }

  output = CreateField(fi, input->mesh());

  if (!output)
  {
    error("Could not allocate output field");
    return false;
  }

  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  VMesh* imesh = input->vmesh();

  if (ifield->is_vector())
  {
    Vector v0, v1, v2, v3;

    int myloc = 0;
    bool not_on_list = false;
    auto num_values = ifield->num_values();

    imesh->synchronize(Mesh::ELEM_NEIGHBORS_E);

    int cnt = 0;

    for (VMesh::Elem::index_type idx = 0; idx < num_values; idx++)
    {
      //calculate neighborhoods
      VMesh::Elem::array_type nci, ncitot, Nlist;
      VMesh::Elem::array_type nci2;
      imesh->get_neighbors(nci, idx);

      ncitot.push_back(idx);

      for (size_t t = 0; t < nci.size(); t++)
      {
        ncitot.push_back(nci[t]);
        imesh->get_neighbors(nci2, nci[t]);

        for (size_t L = 0; L < nci2.size(); L++)
        {
          ncitot.push_back(nci2[L]);
        }

        for (size_t t2 = 0; t2 < nci2.size(); t2++)
        {
          VMesh::Elem::array_type nci3;

          imesh->get_neighbors(nci3, nci2[t2]);
          for (size_t L = 0; L < nci3.size(); L++)
          {
            ncitot.push_back(nci3[L]);
          }
        }
      }

      not_on_list = false;
      for (size_t p1 = 0; p1 < ncitot.size(); p1++)
      {
        not_on_list = false;
        for (size_t p2 = 0; p2 < Nlist.size(); p2++)
        {
          if (ncitot[p1] == Nlist[p2])
          {
            not_on_list = true;
          }

        }
        if (!not_on_list)
        {
          Nlist.push_back(ncitot[p1]);
        }
      }

      std::vector<double> angles, original;
      ifield->get_value(v0, idx);
      for (size_t q = 0; q < Nlist.size(); q++)
      {
        auto a = Nlist[q];
        ifield->get_value(v1, a);
        if (v0.length()*v1.length() == 0)
        {
          angles.push_back(0);
          original.push_back(0);
        }
        else
        {
          auto gdot = Dot(v0, v1);
          auto m1 = v0.length();
          auto m2 = v1.length();
          auto angle = (gdot / (m1*m2));
          angles.push_back(angle);
          original.push_back(angle);
        }
      }

      sort(angles.begin(), angles.end());
      auto middle = (int)((angles.size() + 1) / 2);
      for (size_t k = 0; k < original.size(); k++)
      {
        if (original[k] == angles[middle])
        {
          myloc = k;
          k = original.size();
        }
      }

      auto b = Nlist[myloc];
      ifield->get_value(v2, b);

      ofield->set_value(v2, idx);

      cnt++;
      if (cnt == 200)
      {
        cnt = 0;
        update_progress_max(idx, num_values);
      }
    }
  }
  return (true);
}

AlgorithmOutput SmoothVecFieldMedianAlgo::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);

  FieldHandle outputField;
  if (!runImpl(field, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;
  return output;
}
