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

   Author:               Moritz Dannhauer
   Last Modification:    March 16 2014 (ported from SCIRun4)
   TODO:                 Nrrd aoutput
*/

#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromElemToNode.h>
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

template <class DATA>
bool
  MapFieldDataFromElemToNodeT(const MapFieldDataFromElemToNodeAlgo *algo,
  FieldHandle& input,
  FieldHandle& output)
{
  std::string method = algo->getOption(MapFieldDataFromElemToNodeAlgo::Method);

  VField *ifield = input->vfield();
  VField *ofield = output->vfield();

  /// Make sure that the data vector has the same length
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

  if (method == "Interpolation")
  {
    algo->remark("Interpolation of piecewise constant data is done by averaging adjoining values");
  }

  if ((method == "Interpolation") || (method == "Average"))
  {
    while (it != eit)
    {
      Interruptible::checkForInterruption();
      mesh->get_elems(elems, *(it));
      size_t nsize = elems.size();
      DATA val(0);
      DATA tval;
      for (size_t p = 0; p < nsize; p++)
      {
        ifield->get_value(tval, elems[p]);
        val += tval;
      }
      val = static_cast<DATA>(val*(1.0 / static_cast<double>(nsize)));
      ofield->set_value(val, *(it));
      ++it;
      cnt++;
      if (cnt == 1000)
      {
        cnt = 0; c += 1000;
        algo->update_progress(c / sz);
      }
    }
  }
  else if (method == "Max")
  {
    while (it != eit)
    {
      Interruptible::checkForInterruption();
      mesh->get_elems(elems, *(it));
      size_t nsize = elems.size();
      DATA val(0);
      DATA tval(0);
      if (nsize > 0)
      {
        ifield->get_value(val, elems[0]);
        for (size_t p = 1; p < nsize; p++)
        {
          ifield->get_value(tval, elems[p]);
          if (tval > val) val = tval;
        }
      }
      ofield->set_value(val, *(it));
      ++it;
      cnt++;
      if (cnt == 1000)
      {
        cnt = 0; c += 1000;
        algo->update_progress(c / sz);
      }
    }
  }
  else if (method == "Min")
  {
    while (it != eit)
    {
      Interruptible::checkForInterruption();
      mesh->get_elems(elems, *it);
      size_t nsize = elems.size();
      DATA val(0);
      DATA tval(0);
      if (nsize > 0)
      {
        ifield->get_value(val, elems[0]);
        for (size_t p = 1; p < nsize; p++)
        {
          ifield->value(tval, elems[p]);
          if (tval < val) val = tval;
        }
      }
      ofield->set_value(val, *(it));
      ++it;
      cnt++;
      if (cnt == 1000)
      {
        cnt = 0; c += 1000;
        algo->update_progress(c / sz);
      }
    }
  }
  else if (method == "Sum")
  {
    while (it != eit)
    {
      Interruptible::checkForInterruption();
      mesh->get_elems(elems, *(it));
      size_t nsize = elems.size();
      DATA val(0);
      DATA tval(0);
      for (size_t p = 0; p < nsize; p++)
      {
        ifield->get_value(tval, elems[p]);
        val += tval;
      }
      ofield->set_value(val, *(it));
      ++it;
      cnt++;
      if (cnt == 1000)
      {
        cnt = 0; c += 1000;
        algo->update_progress(c / sz);
      }
    }
  }
  else if (method == "Median")
  {
    std::vector<DATA> valarray;
    while (it != eit)
    {
      Interruptible::checkForInterruption();
      mesh->get_elems(elems, *(it));
      size_t nsize = elems.size();
      valarray.resize(nsize);
      for (size_t p = 0; p < nsize; p++)
      {
        ifield->get_value(valarray[p], elems[p]);
      }
      sort(valarray.begin(), valarray.end());
      int idx = static_cast<int>((valarray.size() / 2));
      ofield->set_value(valarray[idx], *(it));
      ++it;
      cnt++;
      if (cnt == 1000)
      {
        cnt = 0; c += 1000;
        algo->update_progress(c / sz);
      }
    }
  }
  else
  {
    algo->remark("Method is not implemented!");
    return false;
  }

  return true;
}


MapFieldDataFromElemToNodeAlgo::MapFieldDataFromElemToNodeAlgo()
{
  addOption(Method,"Interpolation","Interpolation|Average|Min|Max|Sum|Median|None");
}

AlgorithmParameterName MapFieldDataFromElemToNodeAlgo::Method("Method");

AlgorithmOutput MapFieldDataFromElemToNodeAlgo::run(const AlgorithmInput& input) const
{
  auto input_field = input.get<Field>(Variables::InputField);

  FieldHandle output_field;
  output_field = runImpl(input_field);

  AlgorithmOutput output;
  output[Variables::OutputField] = output_field;

  return output;
}

/// Function call to convert data from Field into Matrix data
FieldHandle MapFieldDataFromElemToNodeAlgo::runImpl(FieldHandle input_field) const
{
  FieldHandle output;

  if (!input_field)
  {
    THROW_ALGORITHM_INPUT_ERROR("Input field is not allocated");
  }

  FieldInformation fi(input_field);
  FieldInformation fo(input_field);

  if (fi.is_lineardata())
  {
    output = input_field;
    remark("Data is already at the nodes");
    return output;
  }

  if (!(fi.is_constantdata()))
  {
    THROW_ALGORITHM_INPUT_ERROR("The input data needs to be located at the elements");
  }

  fo.make_lineardata();

  output = CreateField(fo,input_field->mesh());

  if (!output)
  {
    THROW_ALGORITHM_INPUT_ERROR("output field cannot be allocated");
  }

  if (input_field->vfield()->is_signed_integer())
  {
    if(!MapFieldDataFromElemToNodeT<int>(this,input_field,output))
    {
      THROW_ALGORITHM_INPUT_ERROR("output int field cannot be allocated");
    }
  }
  else if (input_field->vfield()->is_unsigned_integer())
  {
    if(!MapFieldDataFromElemToNodeT<unsigned int>(this,input_field,output))
    {
      THROW_ALGORITHM_INPUT_ERROR("output uint field cannot be allocated");
    }
  } else if (input_field->vfield()->is_scalar())
  {
    if (!MapFieldDataFromElemToNodeT<double>(this,input_field,output))
    {
      THROW_ALGORITHM_INPUT_ERROR("output scalar field cannot be allocated");
    }
  } else if (input_field->vfield()->is_vector())
  {
    if (!MapFieldDataFromElemToNodeT<Vector>(this,input_field,output))
    {
      THROW_ALGORITHM_INPUT_ERROR("output vector field cannot be allocated");
    }
  } else if (input_field->vfield()->is_tensor())
  {
    if (!MapFieldDataFromElemToNodeT<Tensor>(this,input_field,output))
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
