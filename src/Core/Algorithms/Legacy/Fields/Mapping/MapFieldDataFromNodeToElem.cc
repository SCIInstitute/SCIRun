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


#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromNodeToElem.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Thread;
using namespace SCIRun;

/// This is the basic algorithm behind the mapping algorithm

template <class DATA>
bool
MapFieldDataFromNodeToElemT(const MapFieldDataFromNodeToElemAlgo* algo,
                            FieldHandle& input,
                            FieldHandle& output)
{

  /// Get the method the user selected.
  /// Since we do a check of valid entries when then user sets the
  /// algorithm, we can assume it is one of the specified ones
   std::string method = algo->getOption(MapFieldDataFromNodeToElemAlgo::Method);
  /// Get pointers to the virtual interfaces of the fields
  /// We need these to obtain the data values

  VField *ifield = input->vfield();
  VField *ofield = output->vfield();

  /// Make sure that the data vector has the same length
  ofield->resize_fdata();

  VMesh* mesh = input->vmesh();

  VMesh::Elem::array_type elems;
  VMesh::Elem::iterator it, eit;
  VMesh::Elem::size_type sz;

  VMesh::Node::array_type nodearray;

  mesh->begin(it);
  mesh->end(eit);
  mesh->size(sz);
  index_type cnt = 0, c = 0;

  if ((method == "Average") || (method == "Interpolation"))
  {
    DATA tval(0);
    while (it != eit)
    {
      Interruptible::checkForInterruption();
      mesh->get_nodes(nodearray, *it);
      size_t nsize = nodearray.size();
      DATA val(0);
      for (size_t p = 0; p < nsize; p++)
      {
        ifield->get_value(tval,nodearray[p]);
        val += tval;
      }

      val = static_cast<DATA>(val * static_cast<double>((1.0 / static_cast<double>(nsize))));
      ofield->set_value(val, *it);
      ++it;
      cnt++;
      if (cnt == 1000)
      {
        cnt = 0; c += 1000;
        algo->update_progress_max(c, sz);
      }
    }
  }
  else if (method == "Max")
  {
    while (it != eit)
    {
      Interruptible::checkForInterruption();
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
        algo->update_progress_max(c, sz);
      }
    }
  }
  else if (method == "Min")
  {
    DATA tval(0);
    while (it != eit)
    {
      Interruptible::checkForInterruption();
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
        algo->update_progress_max(c, sz);
      }
    }
  }
  else if (method == "Sum")
  {
    DATA tval(0);
    while (it != eit)
    {
      Interruptible::checkForInterruption();
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
        cnt = 0; c += 1000;
        algo->update_progress_max(c, sz);
      }
    }
  }
  else if (method == "Median")
  {
    std::vector<DATA> valarray;
    while (it != eit)
    {
      Interruptible::checkForInterruption();
      mesh->get_nodes(nodearray, *it);
      size_t nsize = nodearray.size();
      valarray.resize(nsize);
      for (size_t p = 0; p < nsize; p++)
      {
        ifield->get_value(valarray[p], nodearray[p]);
      }
      sort(valarray.begin(), valarray.end());
      int idx = static_cast<int>((valarray.size() / 2));
      ofield->set_value(valarray[idx], *it);
      ++it;
      cnt++;
      if (cnt == 1000)
      {
        cnt = 0; c += 1000;
        algo->update_progress_max(c, sz);
      }
    }
  }
  else
    return false;

  return true;
}

MapFieldDataFromNodeToElemAlgo::MapFieldDataFromNodeToElemAlgo()
{
  addOption(Method,"Interpolation","Interpolation|Average|Min|Max|Sum|Median|None");
}

AlgorithmParameterName MapFieldDataFromNodeToElemAlgo::Method("Method");

AlgorithmOutput MapFieldDataFromNodeToElemAlgo::run(const AlgorithmInput& input) const
{
  auto input_field = input.get<Field>(Variables::InputField);

  FieldHandle output_field;
  output_field = runImpl(input_field);

  AlgorithmOutput output;
  output[Variables::OutputField] = output_field;

  return output;
}

FieldHandle MapFieldDataFromNodeToElemAlgo::runImpl(FieldHandle input_field) const
{
  FieldHandle output;

  if (!input_field)
  {
    THROW_ALGORITHM_INPUT_ERROR("Input field is not allocated");
  }

  FieldInformation fi(input_field);
  FieldInformation fo(input_field);

  fo.make_lineardata();

  if (fi.is_constantdata())
  {
    output = input_field;
    remark(" Data is already at the elements");
    return output;
  }

  if (!(fi.is_lineardata()))
  {
    THROW_ALGORITHM_INPUT_ERROR(" This function needs to have data at the nodes ");
  }

  fo.make_constantdata();

  output = CreateField(fo, input_field->mesh());

  if (!output)
  {
    THROW_ALGORITHM_INPUT_ERROR("output field cannot be allocated");
  }

  if (input_field->vfield()->is_signed_integer())
  {
    if (!MapFieldDataFromNodeToElemT<int>(this, input_field, output))
    {
      THROW_ALGORITHM_INPUT_ERROR("output int field cannot be allocated");
    }
  }
  else if (input_field->vfield()->is_unsigned_integer())
  {
    if (!MapFieldDataFromNodeToElemT<unsigned int>(this, input_field, output))
    {
      THROW_ALGORITHM_INPUT_ERROR("output uint field cannot be allocated");
    }
  }
  else if (input_field->vfield()->is_scalar())
  {
    if (!MapFieldDataFromNodeToElemT<double>(this, input_field, output))
    {
      THROW_ALGORITHM_INPUT_ERROR("output scalar field cannot be allocated");
    }
  }
  else if (input_field->vfield()->is_vector())
  {
    if (!MapFieldDataFromNodeToElemT<Vector>(this, input_field, output))
    {
      THROW_ALGORITHM_INPUT_ERROR("output vector field cannot be allocated");
    }
  }
  else if (input_field->vfield()->is_tensor())
  {
    if (!MapFieldDataFromNodeToElemT<Tensor>(this, input_field, output))
    {
      THROW_ALGORITHM_INPUT_ERROR("output tensor field cannot be allocated");
    }
  }
  else
  {
    THROW_ALGORITHM_INPUT_ERROR(" Unknown field data type ");
  }

  return output;
}
