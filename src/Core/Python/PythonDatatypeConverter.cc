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
#include <Core/Datatypes/MatrixTypeConversions.h>

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
  boost::python::dict toPythonList(const SparseRowMatrixGeneric<T>& sparse)
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
    rows.append(sparse.nonZeros());

    boost::python::dict list;
    list["nrows"] = sparse.nrows();
    list["ncols"] = sparse.ncols();
    list["rows"] = rows;
    list["columns"] = columns;
    list["values"] = values;
    return list;
  }
}

boost::python::dict SCIRun::Core::Python::wrapDatatypesInMap(const std::vector<Datatypes::MatrixHandle>& matrices,
  const std::vector<FieldHandle>& fields, const std::vector<Datatypes::StringHandle>& strings)
{
  boost::python::dict d;
  int i = 0;
  for (const auto& m : matrices)
  {
    if (auto dense = castMatrix::toDense(m))
      d[i++] = convertMatrixToPython(dense);
    else if (auto sparse = castMatrix::toSparse(m))
      d[i++] = convertMatrixToPython(sparse);
  }
  for (const auto& f : fields)
    d[i++] = convertFieldToPython(f);
  for (const auto& s : strings)
    d[i++] = convertStringToPython(s);
  return d;
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
    // std::cout << "Field: " << fieldName << std::endl;
    switch (subField.gettype())
    {
    case matfilebase::miUINT8:
    {
      auto str = subField.getstring();
      matlabStructure[fieldName] = str;
      break;
    }
    case matfilebase::miUINT32:
    {
      std::vector<unsigned int> v;
      subField.getnumericarray(v);
      if (1 != subField.getm() && 1 != subField.getn())
        matlabStructure[fieldName] = toPythonListOfLists(v, subField.getn(), subField.getm());
      else
        matlabStructure[fieldName] = toPythonList(v);
      break;
    }
    case matfilebase::miDOUBLE:
    {
      std::vector<double> v;
      subField.getnumericarray(v);
      // std::cout << "miDOUBLE " << subField.getm() << "x" << subField.getn() << "\n";
      // std::copy(v.begin(), v.end(), std::ostream_iterator<double>(std::cout, " "));
      // std::cout << "\n...\n";
      if (1 != subField.getm() && 1 != subField.getn())
        matlabStructure[fieldName] = toPythonListOfLists(v, subField.getn(), subField.getm());
      else
        matlabStructure[fieldName] = toPythonList(v);
      break;
    }
    default:
      std::cout << "some other array: " << fieldName << " of type " << subField.gettype() << std::endl;
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

boost::python::dict SCIRun::Core::Python::convertMatrixToPython(SparseRowMatrixHandle matrix)
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

std::set<std::string> SparseRowMatrixExtractor::validKeys_ = {"rows", "columns", "values", "nrows", "ncols"};

bool SparseRowMatrixExtractor::check() const
{
  boost::python::extract<boost::python::dict> e(object_);
  if (!e.check())
    return false;

  auto dict = e();
  auto length = len(dict);
  if (validKeys_.size() != length)
    return false;

  auto keys = dict.keys();
  auto values = dict.values();

  for (int i = 0; i < length; ++i)
  {
    boost::python::extract<std::string> key_i(keys[i]);
    if (!key_i.check())
      return false;

    if (validKeys_.find(key_i()) == validKeys_.end())
      return false;

    boost::python::extract<boost::python::list> value_i_list(values[i]);
    boost::python::extract<size_t> value_i_int(values[i]);
    if (!value_i_int.check() && !value_i_list.check())
      return false;
  }

  return true;
}

DatatypeHandle SparseRowMatrixExtractor::operator()() const
{
  SparseRowMatrixHandle sparse;
  std::vector<index_type> rows, columns;
  std::vector<double> matrixValues;

  boost::python::extract<boost::python::dict> e(object_);
  auto pyMatlabDict = e();

  auto length = len(pyMatlabDict);

  auto keys = pyMatlabDict.keys();
  auto values = pyMatlabDict.values();
  size_t nrows, ncols;

  for (int i = 0; i < length; ++i)
  {
    boost::python::extract<std::string> key_i(keys[i]);

    boost::python::extract<boost::python::list> value_i_list(values[i]);
    auto fieldName = key_i();
    if (fieldName == "rows")
    {
      rows = to_std_vector<index_type>(value_i_list());
    }
    else if (fieldName == "columns")
    {
      columns = to_std_vector<index_type>(value_i_list());
    }
    else if (fieldName == "nrows")
    {
      boost::python::extract<size_t> e(values[i]);
      nrows = e();
    }
    else if (fieldName == "ncols")
    {
      boost::python::extract<size_t> e(values[i]);
      ncols = e();
    }
    else if (fieldName == "values")
    {
      matrixValues = to_std_vector<double>(value_i_list());
    }
  }

  if (!rows.empty() && !columns.empty() && !matrixValues.empty())
  {
    auto nnz = matrixValues.size();
    return boost::make_shared<SparseRowMatrix>(nrows, ncols, &rows[0], &columns[0], &matrixValues[0], nnz);
  }

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

namespace
{
  matlabarray getPythonFieldDictionaryValue(const boost::python::extract<std::string>& strExtract, const boost::python::extract<boost::python::list>& listExtract)
  {
    matlabarray value;
    if (strExtract.check())
    {
      value.createstringarray();
      auto strData = strExtract();
      //std::cout << "\tsetting string field: " << strData << std::endl;
      value.setstring(strData);
    }
    else if (listExtract.check())
    {
      auto list = listExtract();
      //std::cout << "\tTODO: convert inner lists: " << len(list) << std::endl;
      if (1 == len(list))
      {
        boost::python::extract<double> e(list[0]);
        if (e.check())
          value.createdoublescalar(e());
        else
          std::cerr << "scalar value not readable as double" << std::endl;
      }
      else if (len(list) > 1)
      {
        boost::python::extract<boost::python::list> twoDlistExtract(list[0]);
        if (twoDlistExtract.check())
        {
          std::vector<int> dims = { static_cast<int>(len(list[0])), static_cast<int>(len(list)) };
          auto vectorOfLists = to_std_vector<boost::python::list>(list);
          std::vector<std::vector<double>> vv;
          std::transform(vectorOfLists.begin(), vectorOfLists.end(), std::back_inserter(vv), [](const boost::python::list& inner) { return to_std_vector<double>(inner); });
          std::vector<double> flattenedValues(dims[0] * dims[1]);  //TODO: fill from py list-of-lists
          flatten(vv.begin(), vv.end(), flattenedValues.begin());
          value.createdoublematrix(flattenedValues, dims);
        }
        else // 1-D list
        {
          value.createdoublevector(to_std_vector<double>(list));
        }
      }
    }
    return value;
  }
}

DatatypeHandle FieldExtractor::operator()() const
{
  matlabarray ma;
  matlabconverter mc(nullptr);
  mc.converttostructmatrix();

  boost::python::extract<boost::python::dict> e(object_);
  auto pyMatlabDict = e();

  auto length = len(pyMatlabDict);

  auto keys = pyMatlabDict.keys();
  auto values = pyMatlabDict.values();
  ma.createstructarray();

  for (int i = 0; i < length; ++i)
  {
    boost::python::extract<std::string> key_i(keys[i]);

    boost::python::extract<std::string> value_i_string(values[i]);
    boost::python::extract<boost::python::list> value_i_list(values[i]);
    auto fieldName = key_i();
    //std::cout << "setting field " << fieldName << std::endl;
    ma.setfield(0, fieldName, getPythonFieldDictionaryValue(value_i_string, value_i_list));
  }

  FieldHandle field;
  mc.mlArrayTOsciField(ma, field);
  return field;
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
  {
    SparseRowMatrixExtractor e(object);
    if (e.check())
    {
      return makeDatatypeVariable(e);
    }
  }
  //{
  //  detail::DenseColumnMatrixExtractor e(object);
  //  if (e.check())
  //  {
  //    return makeDatatypeVariable(e);
  //  }
  //}
  {
    FieldExtractor e(object);
    if (e.check())
    {
      return makeDatatypeVariable(e);
    }
  }
  std::cerr << "No known conversion from python object to C++ object" << std::endl;
  return Variable();
}

boost::python::object SCIRun::Core::Python::convertVariableToPythonObject(const Variable& var)
{
  if (var.name().name() == "string")
  {
    return boost::python::object { var.toString() };
  }
  if (var.name().name() == "int")
  {
    return boost::python::object { var.toInt() };
  }
  if (var.name().name() == "double")
  {
    return boost::python::object { var.toDouble() };
  }
  if (var.name().name() == "bool")
  {
    return boost::python::object { var.toBool() };
  }
  return {};
}

template <class Extractor>
std::string getLabel()
{
  boost::python::object empty;
  Extractor dmc(empty);
  return dmc.label();
}

std::string Core::Python::pyDenseMatrixLabel()
{
  return getLabel<DenseMatrixExtractor>();
}

std::string Core::Python::pySparseRowMatrixLabel()
{
  return getLabel<SparseRowMatrixExtractor>();
}

std::string Core::Python::pyFieldLabel()
{
  return getLabel<FieldExtractor>();
}

#endif
