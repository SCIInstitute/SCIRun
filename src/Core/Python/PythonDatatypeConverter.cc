/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

/// @todo Documentation Core/Python/PythonInterpreter.cc

#ifdef BUILD_WITH_PYTHON
#ifdef _MSC_VER
//#pragma warning( push )
#pragma warning( disable: 4244 )
#endif

#include <Core/Python/PythonDatatypeConverter.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Matlab/matlabconverter.h>

using namespace SCIRun;
using namespace SCIRun::Core::Python;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::MatlabIO;

namespace
{
  template <class T>
  boost::python::list toPythonList(const DenseMatrixGeneric<T>& dense)
  {
    boost::python::list list;
    for (int i = 0; i < dense.nrows(); ++i)
    {
      boost::python::list row;
      for (int j = 0; j < dense.ncols(); ++j)
        row.append(dense(i, j));
      list.append(row);
    }
    return list;
  }

  template <class T>
  boost::python::list toPythonList(const SparseRowMatrixGeneric<T>& sparse)
  {
    boost::python::list rows, columns, values;

    for (int i = 0; i < sparse.nonZeros(); ++i)
    {
      values.append(sparse.valuePtr()[i]);
    }
    for (int i = 0; i < sparse.nonZeros(); ++i)
    {
      columns.append(sparse.innerIndexPtr()[i]);
    }
    for (int i = 0; i < sparse.outerSize(); ++i)
    {
      rows.append(sparse.outerIndexPtr()[i]);
    }

    boost::python::list list;
    list.append(rows);
    list.append(columns);
    list.append(values);
    return list;
  }
}

boost::python::dict SCIRun::Core::Python::convertFieldToPython(FieldHandle field)
{
  matlabarray ma;
  matlabconverter mc(nullptr);
  mc.converttostructmatrix();
  mc.sciFieldTOmlArray(field, ma);
  boost::python::dict matlabStructure;

  for (const auto& fieldName : ma.getfieldnames())
  {
    auto subField = ma.getfield(0, fieldName);
    switch (subField.gettype())
    {
    case matfilebase::miUINT8:
    {
      auto str = subField.getstring();
      matlabStructure[fieldName] = str;
      break;
    }
    case matfilebase::miDOUBLE:
    {
      std::vector<double> v;
      subField.getnumericarray(v);
      if (1 != subField.getm() && 1 != subField.getn())
        matlabStructure[fieldName] = toPythonListOfLists(v, subField.getn(), subField.getm());
      else
        matlabStructure[fieldName] = toPythonList(v);
      break;
    }
    default:
      std::cout << "some other array: " << std::endl;
      break;
    }
  }
  return matlabStructure;
}

boost::python::list SCIRun::Core::Python::convertMatrixToPython(DenseMatrixHandle matrix)
{
  if (matrix)
    return ::toPythonList(*matrix);
  return {};
}

boost::python::object SCIRun::Core::Python::convertMatrixToPython(SparseRowMatrixHandle matrix)
{
  if (matrix)
    return ::toPythonList(*matrix);
  return {};
}

boost::python::object SCIRun::Core::Python::convertStringToPython(StringHandle str)
{
  if (str)
  {
    boost::python::object obj(std::string(str->value()));
    return obj;
  }
  return {};
}

bool DenseMatrixExtractor::check() const 
{
  boost::python::extract<boost::python::list> e(object_);
  if (!e.check())
    return false;

  auto list = e();
  auto length = len(list);
  if (length > 0)
  {
    boost::python::extract<boost::python::list> firstRow(list[0]);
    return firstRow.check();
  }
  return false;
}

DatatypeHandle DenseMatrixExtractor::operator()() const
{
  DenseMatrixHandle dense;
  boost::python::extract<boost::python::list> e(object_);
  if (e.check())
  {
    auto list = e();
    auto length = len(list);
    bool copyValues = false;
        
    if (length > 0)
    {
      boost::python::extract<boost::python::list> firstRow(list[0]);
      if (firstRow.check())
      {
        copyValues = true;
        dense.reset(new DenseMatrix(length, len(firstRow)));
      }
    }
    else
    {
      dense.reset(new DenseMatrix(0, 0));
    }
    if (copyValues)
    {
      for (int i = 0; i < length; ++i)
      {
        boost::python::extract<boost::python::list> rowList(list[i]);
        if (rowList.check())
        {
          auto row = rowList();
          if (len(row) != dense->ncols())
            throw std::invalid_argument("Attempted to convert into dense matrix but row lengths are not all equal.");
          for (int j = 0; j < len(row); ++j)
          {
            (*dense)(i, j) = boost::python::extract<double>(row[j]);
          }
        }
      }
    }
  }
  return dense;
}

bool SparseRowMatrixExtractor::check() const
{
  return false;
}

DatatypeHandle SparseRowMatrixExtractor::operator()() const
{
  return nullptr;
}

bool FieldExtractor::check() const
{
  boost::python::extract<boost::python::dict> e(object_);
  if (!e.check())
    return false;

  auto dict = e();
  auto length = len(dict);
  if (0 == length)
    return false;
  
  auto keys = dict.keys();
  auto values = dict.values();

  for (int i = 0; i < length; ++i)
  {
    boost::python::extract<std::string> key_i(keys[i]);
    if (!key_i.check())
      return false;

    boost::python::extract<std::string> value_i_string(values[i]);
    boost::python::extract<boost::python::list> value_i_list(values[i]);
    if (!value_i_string.check() && !value_i_list.check())
      return false;
  }

  return true;
}

DatatypeHandle FieldExtractor::operator()() const
{
  return nullptr;
}

namespace 
{
  Variable makeDatatypeVariable(const DatatypePythonExtractor& extractor)
  {
    return Variable(Name(extractor.label()), extractor(), Variable::DATATYPE_VARIABLE);
  }
}

Variable SCIRun::Core::Python::convertPythonObjectToVariable(const boost::python::object& object)
{
  /// @todo: yucky
  {
    boost::python::extract<int> e(object);
    if (e.check())
    {
      return makeVariable("int", e());
    }
  }
  {
    boost::python::extract<double> e(object);
    if (e.check())
    {
      return makeVariable("double", e());
    }
  }
  {
    boost::python::extract<std::string> e(object);
    if (e.check())
    {
      return makeVariable("string", e());
    }
  }
  {
    boost::python::extract<bool> e(object);
    if (e.check())
    {
      return makeVariable("bool", e());
    }
  }
  {
    DenseMatrixExtractor e(object);
    if (e.check())
    {
      return makeDatatypeVariable(e);
    }
  }
  //{
  //  detail::SparseRowMatrixExtractor e(object);
  //  if (e.check())
  //  {
  //    return makeDatatypeVariable(e);
  //  }
  //}
  //{
  //  detail::DenseColumnMatrixExtractor e(object);
  //  if (e.check())
  //  {
  //    return makeDatatypeVariable(e);
  //  }
  //}
  //{
  //  detail::FieldExtractor e(object);
  //  if (e.check())
  //  {
  //    return makeDatatypeVariable(e);
  //  }
  //}
  std::cerr << "No known conversion from python object to C++ object" << std::endl;
  return Variable();
}

#endif