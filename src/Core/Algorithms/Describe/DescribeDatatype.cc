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


#include <Core/Algorithms/Describe/DescribeDatatype.h>
#include <Core/Algorithms/Field/ReportFieldInfoAlgorithm.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Algorithms/Math/ReportComplexMatrixInfo.h>

using namespace SCIRun::Core::Algorithms::General;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

std::string DescribeDatatype::describe(const DatatypeHandle& data) const
{
  if (!data)
    return "[null data]";

  auto str = boost::dynamic_pointer_cast<String>(data);
  if (str)
  {
    auto value = str->value();
    return "[String Data]  Contents:\n" + (!value.empty() ? value : "(empty string)");
  }

  auto mat = boost::dynamic_pointer_cast<Matrix>(data);
  if (mat)
  {
    ReportMatrixInfoAlgorithm algo;
    auto info = algo.runImpl(mat);

    return "[Matrix Data] Info:\n" + ReportMatrixInfoAlgorithm::summarize(info);
  }

  auto cmat = boost::dynamic_pointer_cast<ComplexDenseMatrix>(data);
  if (cmat)
  {
    ReportComplexMatrixInfoAlgo algo;
    auto info = algo.runImpl(cmat);

    return "[Complex Matrix Data] Info:\n" + ReportComplexMatrixInfoAlgo::summarize(info);
  }

  auto cmatsp = boost::dynamic_pointer_cast<ComplexSparseRowMatrix>(data);
  if (cmatsp)
  {
    ReportComplexMatrixInfoAlgo algo;
    auto info = algo.runImpl(cmatsp);

    return "[Complex Matrix Data] Info:\n" + ReportComplexMatrixInfoAlgo::summarize(info);
  }

  auto field = boost::dynamic_pointer_cast<Field>(data);
  if (field)
  {
    ReportFieldInfoAlgorithm algo;
    auto info = algo.run(field);

    return "[Field Data] Info:\n" + ReportFieldInfoAlgorithm::summarize(info);
  }

  auto geom = boost::dynamic_pointer_cast<GeometryObject>(data);
  if (geom)
  {
    return "[Geometry Object] ID:\n" + geom->uniqueID();
  }

  return "[Unknown Datatype]";
}
