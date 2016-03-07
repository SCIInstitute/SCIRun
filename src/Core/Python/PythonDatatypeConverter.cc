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

boost::python::object SCIRun::Core::Python::convertFieldToPython(FieldHandle field)
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

boost::python::object SCIRun::Core::Python::convertMatrixToPython(DenseMatrixHandle matrix)
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

namespace detail
{
  class DatatypeExtractor
  {
  public:
    virtual ~DatatypeExtractor() {}
    virtual bool check() const = 0;
    virtual DatatypeHandle operator()() const = 0;
    virtual std::string label() const = 0;
  };

  class DenseMatrixExtractor : public DatatypeExtractor
  {
  public:
    explicit DenseMatrixExtractor(const boost::python::object& object) : object_(object) {}

    virtual bool check() const override
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

    virtual DatatypeHandle operator()() const override
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

    virtual std::string label() const override { return "dense matrix"; }
  private:
    const boost::python::object& object_;
  };

  class SparseRowMatrixExtractor : public DatatypeExtractor
  {
  public:
    explicit SparseRowMatrixExtractor(const boost::python::object& object) : object_(object) {}
    bool check() const;
    DatatypeHandle operator()() const;
    std::string label() const;
  private:
    const boost::python::object& object_;
  };

  class FieldExtractor : public DatatypeExtractor
  {
  public:
    explicit FieldExtractor(const boost::python::object& object) : object_(object) {}
    bool check() const;
    DatatypeHandle operator()() const;
    std::string label() const;
  private:
    const boost::python::object& object_;
  };

  Variable makeDatatypeVariable(const DatatypeExtractor& extractor)
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
    detail::DenseMatrixExtractor e(object);
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
  //  detail::FieldExtractor e(object);
  //  if (e.check())
  //  {
  //    return makeDatatypeVariable(e);
  //  }
  //}
#if 0
  {
    boost::python::extract<boost::python::list> e(object);
    if (e.check())
    {
      auto list = e();
      auto length = len(list);
      bool makeDense;
      DenseMatrixHandle dense;
      if (length > 0)
      {
        boost::python::extract<boost::python::list> firstRow(list[0]);
        if (firstRow.check())
        {
          makeDense = true;
          dense.reset(new DenseMatrix(length, len(firstRow)));
        }
        else
        {
          boost::python::extract<std::string> innerString(list[0]);
          if (innerString.check())
            makeDense = false;
          else
            throw std::invalid_argument("Ill-formed list.");
        }
      }
      else
      {
        throw std::invalid_argument("Empty list.");
      }
      if (makeDense)
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
        Variable x(Name("dense matrix"), dense, Variable::DATATYPE_VARIABLE);
        return x;
      }
      else //sparse
      {
        std::cout << "TODO: sparse matrix conversion" << std::endl;
        return {};
      }
      //TODO: dense column
    }
  }
#endif
  std::cerr << "No known conversion from python object to C++ object" << std::endl;
  return Variable();
}


DenseMatrixHandle SCIRun::Core::Python::extractDenseMatrixFromPython(const boost::python::object& obj)
{
  
  return nullptr;

}

#endif