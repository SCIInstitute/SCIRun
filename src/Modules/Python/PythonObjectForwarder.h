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


#ifndef MODULES_PYTHON_PYTHONOBJECTFORWARDER_H
#define MODULES_PYTHON_PYTHONOBJECTFORWARDER_H

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Python/PythonDatatypeConverter.h>
#include <boost/thread.hpp>
#include <Core/Logging/Log.h>
#include <Modules/Python/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Python
      {
        ALGORITHM_PARAMETER_DECL(PollingIntervalMilliseconds);
        ALGORITHM_PARAMETER_DECL(NumberOfRetries);
        ALGORITHM_PARAMETER_DECL(PythonObject);

#ifdef BUILD_WITH_PYTHON
        template <class PythonModule>
        class PythonObjectForwarderImpl
        {
        public:
          explicit PythonObjectForwarderImpl(PythonModule& module) : module_(module)
          {
            auto state = module.get_state();
            maxTries_ = state->getValue(Parameters::NumberOfRetries).toInt();
            waitTime_ = state->getValue(Parameters::PollingIntervalMilliseconds).toInt();
          }

          template <class StringPort, class MatrixPort, class FieldPort>
          Datatypes::DatatypeHandle waitForOutputFromTransientState(const std::string& transientKey,
            const StringPort& stringPort, const MatrixPort& matrixPort, const FieldPort& fieldPort)
          {
            int tries = 0;
            auto state = module_.get_state();
            auto valueOption = state->getTransientValue(transientKey);

            while (tries < maxTries_ && !valueOption)
            {
              std::ostringstream ostr;
              ostr << module_.id() << " looking up value for " << transientKey << "; attempt #" << (tries + 1) << "/" << maxTries_;
              module_.remark(ostr.str());

              valueOption = state->getTransientValue(transientKey);

              tries++;
              boost::this_thread::sleep(boost::posix_time::milliseconds(waitTime_));
            }

            Datatypes::DatatypeHandle output;

            if (valueOption)
            {
              auto var = Dataflow::Networks::transient_value_cast<Variable>(valueOption);
              //logCritical("ValueOption found, typename is {}", var.name().name());
              if (var.name().name() == "string")
              {
                auto valueStr = var.toString();
                auto strObj = boost::make_shared<Datatypes::String>(!valueStr.empty() ? valueStr : "Empty string or non-string received");
                output = strObj;
                module_.sendOutput(stringPort, strObj);
              }
              else if (var.name().name() == "int")
              {
                auto valueInt = var.toInt();
                output = boost::make_shared<Datatypes::DenseMatrix>(1, 1, valueInt);
                // special case, don't send, just return value
                //module_.sendOutput(matrixPort, output);
              }
              else if (var.name().name() == Core::Python::pyDenseMatrixLabel())
              {
                auto dense = boost::dynamic_pointer_cast<Core::Datatypes::DenseMatrix>(var.getDatatype());
                if (dense)
                {
                  output = dense;
                  module_.sendOutput(matrixPort, dense);
                }
              }
              else if (var.name().name() == Core::Python::pySparseRowMatrixLabel())
              {
                auto sparse = boost::dynamic_pointer_cast<Core::Datatypes::SparseRowMatrix>(var.getDatatype());
                if (sparse)
                {
                  output = sparse;
                  module_.sendOutput(matrixPort, sparse);
                }
              }
              else if (var.name().name() == Core::Python::pyFieldLabel())
              {
                auto field = boost::dynamic_pointer_cast<Core::Datatypes::Field>(var.getDatatype());
                if (field)
                {
                  output = field;
                  module_.sendOutput(fieldPort, field);
                }
              }
            }
            return output;
          }
        private:
          PythonModule& module_;
          int maxTries_, waitTime_;
        };
        #endif
      }
    }
  }


  namespace Modules
  {
    namespace Python
    {
      class SCISHARE PythonObjectForwarder : public SCIRun::Dataflow::Networks::Module,
        public Has3OutputPorts<MatrixPortTag, FieldPortTag, StringPortTag>,
        public HasNoInputPorts
      {
      public:
        PythonObjectForwarder();
        virtual void execute() override;
        virtual void setStateDefaults() override;
        OUTPUT_PORT(0, PythonMatrix, Matrix);
        OUTPUT_PORT(1, PythonField, Field);
        OUTPUT_PORT(2, PythonString, String);

        MODULE_TRAITS_AND_INFO(ModuleHasUI)
      };

    }
  }
}

#endif
