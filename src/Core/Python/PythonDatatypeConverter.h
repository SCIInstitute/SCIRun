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


#ifdef BUILD_WITH_PYTHON
#ifndef CORE_PYTHON_PYTHONDATATYPECONVERTER_H
#define CORE_PYTHON_PYTHONDATATYPECONVERTER_H

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <vector>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Algorithms/Base/Variable.h>


#include <Core/Python/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Python
    {
      template <class T>
      boost::python::list toPythonList(const std::vector<T>& vec)
      {
        boost::python::list list;
        for (const auto& v : vec)
        {
          list.append(v);
        }
        return list;
      }

      template <class T>
      boost::python::list toPythonListOfLists(const std::vector<T>& vec, int dim1, int dim2)
      {
        boost::python::list list;
        auto iter = vec.begin();
        for (int i = 0; i < dim1; ++i)
        {
          boost::python::list row;
          for (int j = 0; j < dim2; ++j)
            row.append(*iter++);
          list.append(row);
        }
        return list;
      }

      template< typename T >
      std::vector< T > to_std_vector(const boost::python::object& iterable)
      {
        return std::vector< T >(boost::python::stl_input_iterator< T >(iterable),
          boost::python::stl_input_iterator< T >());
      }

      template <class ContainerOfIterableIterator, class OutputIterator>
      void flatten(ContainerOfIterableIterator outerBegin, ContainerOfIterableIterator outerEnd, OutputIterator dest)
      {
        while (outerBegin != outerEnd) {
          dest = std::copy(outerBegin->begin(), outerBegin->end(), dest);
          ++outerBegin;
        }
      }

      SCISHARE boost::python::dict convertFieldToPython(FieldHandle field);
      SCISHARE boost::python::list convertMatrixToPython(Datatypes::DenseMatrixHandle matrix);
      SCISHARE boost::python::dict convertMatrixToPython(Datatypes::SparseRowMatrixHandle matrix);
      SCISHARE boost::python::object convertStringToPython(Datatypes::StringHandle str);
      SCISHARE boost::python::dict wrapDatatypesInMap(
        const std::vector<Datatypes::MatrixHandle>& matrices, 
        const std::vector<FieldHandle>& fields,
        const std::vector<Datatypes::StringHandle>& strings);

      SCISHARE Algorithms::Variable convertPythonObjectToVariable(const boost::python::object& object);
      SCISHARE boost::python::object convertVariableToPythonObject(const Algorithms::Variable& object);

      class SCISHARE DatatypePythonExtractor
      {
      public:
        virtual ~DatatypePythonExtractor() {}
        explicit DatatypePythonExtractor(const boost::python::object& object) : object_(object) {}
        virtual bool check() const = 0;
        virtual Datatypes::DatatypeHandle operator()() const = 0;
        virtual std::string label() const = 0;
      protected:
        const boost::python::object& object_;
      };

      class SCISHARE DenseMatrixExtractor : public DatatypePythonExtractor
      {
      public:
        explicit DenseMatrixExtractor(const boost::python::object& object) : DatatypePythonExtractor(object) {}
        virtual bool check() const override;
        virtual Datatypes::DatatypeHandle operator()() const override;
        virtual std::string label() const override { return "dense matrix"; }
      };

      class SCISHARE SparseRowMatrixExtractor : public DatatypePythonExtractor
      {
      public:
        explicit SparseRowMatrixExtractor(const boost::python::object& object) : DatatypePythonExtractor(object) {}
        virtual bool check() const override;
        virtual Datatypes::DatatypeHandle operator()() const override;
        virtual std::string label() const override { return "sparse matrix"; }
      private:
        static std::set<std::string> validKeys_;
      };

      class SCISHARE FieldExtractor : public DatatypePythonExtractor
      {
      public:
        explicit FieldExtractor(const boost::python::object& object) : DatatypePythonExtractor(object) {}
        virtual bool check() const override;
        virtual Datatypes::DatatypeHandle operator()() const override;
        virtual std::string label() const override { return "field"; }
      };

      SCISHARE std::string pyDenseMatrixLabel();
      SCISHARE std::string pySparseRowMatrixLabel();
      SCISHARE std::string pyFieldLabel();
    }
  }
}

#endif
#endif
