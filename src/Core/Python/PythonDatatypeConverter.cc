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
#pragma warning(disable : 4244)
#endif

#include <Core/Datatypes/DenseMatrix.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/String.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Matlab/matlabconverter.h>
#include <Core/Python/PythonDatatypeConverter.h>
#include <Core/Algorithms/Base/VariableHelper.h>
#include <boost/variant/apply_visitor.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Python;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::MatlabIO;

namespace py = boost::python;

namespace {
struct PythonObjectVisitor : boost::static_visitor<py::object>
{
  py::object operator()(const int& v) const { return py::object(v); }
  py::object operator()(const std::string& v) const { return py::object(v); }
  py::object operator()(const double& v) const { return py::object(v); }
  py::object operator()(const bool& v) const { return py::object(v); }
  py::object operator()(const AlgoOption&) const
  {
    THROW_INVALID_ARGUMENT("Conversion to AlgoOption is not implemented.");
  }
  py::object operator()(const VariableList& vl) const
  {
    py::list pyList;
    for (const auto& v : vl)
      pyList.append(boost::apply_visitor(PythonObjectVisitor(), v.value()));
    return std::move(pyList);
  }
};

struct ValueVisitor : boost::static_visitor<Variable::Value>
{
private:
  const py::object& object_;
public:
  explicit ValueVisitor(const py::object& object) : object_(object) {}

  enum class NumberType
  {
    Int,
    Double
  };

  Variable::Value operator()(const int&) const
  {
    return convertNumber(NumberType::Int);
  }

  Variable::Value operator()(const double&) const
  {
    return convertNumber(NumberType::Double);
  }

  Variable::Value operator()(const bool&) const
  {
    if (getClassName(object_) == "bool")
    {
      py::extract<bool> e(object_);
      return e();
    }
    else
      THROW_INVALID_ARGUMENT("The input python object is not a boolean.");
  }

  Variable::Value operator()(const std::string&) const
  {
    if (getClassName(object_) == "str")
    {
      py::extract<std::string> e(object_);
      return e();
    }
    else
      THROW_INVALID_ARGUMENT("The input python object is not a string.");
  }

  Variable::Value operator()(const AlgoOption&) const
  {
    // All variables using AlgoOption go to the string operator so this is never reached
    THROW_INVALID_ARGUMENT("Conversion to AlgoOption is not implemented.");
  }

  Variable::Value operator()(const Variable::List& v) const
  {
    if (getClassName(object_) == "list")
    {
      const auto firstVal = v[0];
      const py::extract<py::list> e(object_);
      auto pyList = e();
      Variable::List newList(py::len(pyList));
      for (auto i = 0; i < py::len(pyList); ++i)
        newList[i] = Variable(firstVal.name(),
                              boost::apply_visitor(ValueVisitor(pyList[i]), firstVal.value()));
      return newList;
    }
    else
      THROW_INVALID_ARGUMENT("The input python object is not a list.");
  }

  Variable::Value convertNumber(NumberType returnType) const
  {
    const auto classname = getClassName(object_);
    if (classname == "int")
    {
      py::extract<int> e(object_);
      const auto objectVal = e();
      switch (returnType)
      {
      case NumberType::Int: return objectVal;
      case NumberType::Double: return static_cast<int>(objectVal);
      }
    }
    else if (classname == "float")
    {
      py::extract<float> e(object_);
      const auto objectVal = e();
      switch (returnType)
      {
      case NumberType::Int: return static_cast<int>(objectVal);
      case NumberType::Double: return static_cast<double>(objectVal);
      }
    }
    else if (classname == "double")
    {
      py::extract<double> e(object_);
      const auto objectVal = e();
      switch (returnType)
      {
      case NumberType::Int: return static_cast<double>(objectVal);
      case NumberType::Double: return objectVal;
      }
    }
    else
      THROW_INVALID_ARGUMENT("The input python object is not a number.");
  }
};

template <class T>
py::list toPythonListDense(const DenseMatrixGeneric<T>& dense)
{
  py::list list;
  for (int i = 0; i < dense.nrows(); ++i)
  {
    py::list row;
    for (int j = 0; j < dense.ncols(); ++j)
      row.append(dense(i, j));
    list.append(row);
  }
  return list;
}

template <class T>
py::dict toPythonListSparse(const SparseRowMatrixGeneric<T>& sparse)
{
  py::list rows, columns, values;

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

  py::dict list;
  list["nrows"] = sparse.nrows();
  list["ncols"] = sparse.ncols();
  list["rows"] = rows;
  list["columns"] = columns;
  list["values"] = values;
  return list;
}
}

py::dict SCIRun::Core::Python::wrapDatatypesInMap(
    const std::vector<Datatypes::MatrixHandle>& matrices, const std::vector<FieldHandle>& fields,
    const std::vector<Datatypes::StringHandle>& strings)
{
  py::dict d;
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

py::dict SCIRun::Core::Python::convertFieldToPython(FieldHandle field)
{
  matlabarray ma;
  matlabconverter mc(nullptr);
  mc.converttostructmatrix();
  mc.sciFieldTOmlArray(field, ma);
  py::dict matlabStructure;

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
      std::cout << "some other array: " << fieldName << " of type " << subField.gettype()
                << std::endl;
      break;
    }
  }
  return matlabStructure;
}

py::list SCIRun::Core::Python::convertMatrixToPython(DenseMatrixHandle matrix)
{
  if (matrix) return ::toPythonListDense(*matrix);
  return {};
}

py::dict SCIRun::Core::Python::convertMatrixToPython(SparseRowMatrixHandle matrix)
{
  if (matrix) return ::toPythonListSparse(*matrix);
  return {};
}

py::object SCIRun::Core::Python::convertStringToPython(StringHandle str)
{
  if (str)
  {
    py::object obj(std::string(str->value()));
    return obj;
  }
  return {};
}

bool DenseMatrixExtractor::check() const
{
  py::extract<py::list> e(object_);
  if (!e.check()) return false;

  auto list = e();
  auto length = len(list);
  if (length > 0)
  {
    py::extract<py::list> firstRow(list[0]);
    return firstRow.check();
  }
  return false;
}

DatatypeHandle DenseMatrixExtractor::operator()() const
{
  DenseMatrixHandle dense;
  py::extract<py::list> e(object_);
  if (e.check())
  {
    auto list = e();
    auto length = len(list);
    bool copyValues = false;

    if (length > 0)
    {
      py::extract<py::list> firstRow(list[0]);
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
        py::extract<py::list> rowList(list[i]);
        if (rowList.check())
        {
          auto row = rowList();
          if (len(row) != dense->ncols())
            throw std::invalid_argument(
                "Attempted to convert into dense matrix but row lengths are not all equal.");
          for (int j = 0; j < len(row); ++j)
          {
            (*dense)(i, j) = py::extract<double>(row[j]);
          }
        }
      }
    }
  }
  return dense;
}

std::set<std::string> SparseRowMatrixExtractor::validKeys_ = {
    "rows", "columns", "values", "nrows", "ncols"};

bool SparseRowMatrixExtractor::check() const
{
  py::extract<py::dict> e(object_);
  if (!e.check()) return false;

  auto dict = e();
  auto length = len(dict);
  if (validKeys_.size() != length) return false;

  auto keys = dict.keys();
  auto values = dict.values();

  for (int i = 0; i < length; ++i)
  {
    py::extract<std::string> key_i(keys[i]);
    if (!key_i.check()) return false;

    if (validKeys_.find(key_i()) == validKeys_.end()) return false;

    py::extract<py::list> value_i_list(values[i]);
    py::extract<size_t> value_i_int(values[i]);
    if (!value_i_int.check() && !value_i_list.check()) return false;
  }

  return true;
}

DatatypeHandle SparseRowMatrixExtractor::operator()() const
{
  SparseRowMatrixHandle sparse;
  std::vector<index_type> rows, columns;
  std::vector<double> matrixValues;

  py::extract<py::dict> e(object_);
  auto pyMatlabDict = e();

  auto length = len(pyMatlabDict);

  auto keys = pyMatlabDict.keys();
  auto values = pyMatlabDict.values();
  size_t nrows, ncols;

  for (int i = 0; i < length; ++i)
  {
    py::extract<std::string> key_i(keys[i]);

    py::extract<py::list> value_i_list(values[i]);
    auto fieldName = key_i();
    if (fieldName == "rows") { rows = to_std_vector<index_type>(value_i_list()); }
    else if (fieldName == "columns")
    {
      columns = to_std_vector<index_type>(value_i_list());
    }
    else if (fieldName == "nrows")
    {
      py::extract<size_t> e(values[i]);
      nrows = e();
    }
    else if (fieldName == "ncols")
    {
      py::extract<size_t> e(values[i]);
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
    return makeShared<SparseRowMatrix>(
        nrows, ncols, &rows[0], &columns[0], &matrixValues[0], nnz);
  }

  return nullptr;
}

bool FieldExtractor::check() const
{
  py::extract<py::dict> e(object_);
  if (!e.check()) return false;

  auto dict = e();
  auto length = len(dict);
  if (0 == length) return false;

  auto keys = dict.keys();
  auto values = dict.values();

  for (int i = 0; i < length; ++i)
  {
    py::extract<std::string> key_i(keys[i]);
    if (!key_i.check()) return false;

    py::extract<std::string> value_i_string(values[i]);
    py::extract<py::list> value_i_list(values[i]);
    if (!value_i_string.check() && !value_i_list.check()) return false;
  }

  return true;
}

namespace {
matlabarray getPythonFieldDictionaryValue(
    const py::extract<std::string>& strExtract, const py::extract<py::list>& listExtract)
{
  matlabarray value;
  if (strExtract.check())
  {
    value.createstringarray();
    auto strData = strExtract();
    // std::cout << "\tsetting string field: " << strData << std::endl;
    value.setstring(strData);
  }
  else if (listExtract.check())
  {
    auto list = listExtract();
    // std::cout << "\tTODO: convert inner lists: " << len(list) << std::endl;
    if (1 == len(list))
    {
      py::extract<double> e(list[0]);
      if (e.check())
        value.createdoublescalar(e());
      else
        std::cerr << "scalar value not readable as double" << std::endl;
    }
    else if (len(list) > 1)
    {
      py::extract<py::list> twoDlistExtract(list[0]);
      if (twoDlistExtract.check())
      {
        std::vector<int> dims = {static_cast<int>(len(list[0])), static_cast<int>(len(list))};
        auto vectorOfLists = to_std_vector<py::list>(list);
        std::vector<std::vector<double>> vv;
        std::transform(vectorOfLists.begin(), vectorOfLists.end(), std::back_inserter(vv),
            [](const py::list& inner) { return to_std_vector<double>(inner); });
        std::vector<double> flattenedValues(dims[0] * dims[1]);  // TODO: fill from py list-of-lists
        flatten(vv.begin(), vv.end(), flattenedValues.begin());
        value.createdoublematrix(flattenedValues, dims);
      }
      else  // 1-D list
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

  py::extract<py::dict> e(object_);
  auto pyMatlabDict = e();

  auto length = len(pyMatlabDict);

  auto keys = pyMatlabDict.keys();
  auto values = pyMatlabDict.values();
  ma.createstructarray();

  for (int i = 0; i < length; ++i)
  {
    py::extract<std::string> key_i(keys[i]);

    py::extract<std::string> value_i_string(values[i]);
    py::extract<py::list> value_i_list(values[i]);
    auto fieldName = key_i();
    // std::cout << "setting field " << fieldName << std::endl;
    ma.setfield(0, fieldName, getPythonFieldDictionaryValue(value_i_string, value_i_list));
  }

  FieldHandle field;
  mc.mlArrayTOsciField(ma, field);
  return field;
}

namespace {
  Variable makeDatatypeVariable(const DatatypePythonExtractor& extractor)
{
  return Variable(Name(extractor.label()), extractor(), Variable::DatatypeVariableEnum::DATATYPE_VARIABLE);
}
}

const std::string SCIRun::Core::Python::getClassName(const py::object& object)
{
  const py::extract<py::object> extractor(object);
  return py::extract<std::string>(extractor().attr("__class__").attr("__name__"));
}

Variable SCIRun::Core::Python::convertPythonObjectToVariable(const py::object& object)
{
  {
    py::extract<int> e(object);
    if (e.check())
      return makeVariable("int", e());
  }
  {
    py::extract<double> e(object);
    if (e.check())
      return makeVariable("double", e());
  }
  {
    py::extract<std::string> e(object);
    if (e.check())
      return makeVariable("string", e());
  }
  {
    py::extract<bool> e(object);
    if (e.check())
      return makeVariable("bool", e());
  }
  {
    py::extract<py::list> e(object);
    if (e.check())
    {
      auto pyList = e();
      Variable::List newList(py::len(pyList));
      for (auto i = 0; i < py::len(pyList); ++i)
        newList[i] = convertPythonObjectToVariable(pyList[i]);
      return makeVariable("list", newList);
    }
  }
  {
    SparseRowMatrixExtractor e(object);
    if (e.check())
      return makeDatatypeVariable(e);
  }
  {
    FieldExtractor e(object);
    if (e.check())
      return makeDatatypeVariable(e);
  }
  std::cerr << "No known conversion from python object to C++ object" << std::endl;
  return Variable();
}

Variable SCIRun::Core::Python::convertPythonObjectToVariableWithTypeInference(
    const py::object& object, const Variable& var)
{
  return Variable(var.name(), boost::apply_visitor(ValueVisitor(object), var.value()));
}

py::object SCIRun::Core::Python::convertVariableToPythonObject(const Variable& var)
{
  return boost::apply_visitor(PythonObjectVisitor(), var.value());
}

py::object SCIRun::Core::Python::convertTransientVariableToPythonObject(
    const std::optional<boost::any>& v)
{
  if (transient_value_check<Variable>(v))
    return convertVariableToPythonObject(transient_value_cast<Variable>(v));
  else
    return py::object();
}

template <class Extractor>
std::string getLabel()
{
  py::object empty;
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
