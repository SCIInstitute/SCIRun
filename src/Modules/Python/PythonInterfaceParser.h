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


#ifndef MODULES_PYTHON_PYTHONINTERFACEPARSER_H
#define MODULES_PYTHON_PYTHONINTERFACEPARSER_H

#include <Dataflow/Network/ModuleStateInterface.h>
#include <Modules/Python/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Python
      {
        constexpr const char* matlabDelimiter { "%%" };

        struct SCISHARE PythonCodeBlock
        {
          std::string code;
          bool isMatlab;
        };

        using PythonCode = std::list<PythonCodeBlock>;

        class SCISHARE InterfaceWithPythonCodeTranslator
        {
        public:
          virtual ~InterfaceWithPythonCodeTranslator() {}
          virtual PythonCodeBlock translate(const std::string& code) const = 0;
          virtual void updatePorts(const std::vector<std::string>& portIds) = 0;
        };

        using ModuleIdGetter = std::function<std::string()>;

        class SCISHARE InterfaceWithPythonCodeTranslatorImpl : public InterfaceWithPythonCodeTranslator
        {
        public:
          InterfaceWithPythonCodeTranslatorImpl(ModuleIdGetter moduleId,
            const Dataflow::Networks::ModuleStateHandle& state, const std::vector<AlgorithmParameterName>& outputNamesToCheck);

          void updatePorts(const std::vector<std::string>& portIds) override { portIds_ = portIds; }
          PythonCodeBlock translate(const std::string& code) const override;

          PythonCodeBlock translateIOSyntax(const PythonCodeBlock& code) const;

          std::string translateInputSyntax(const std::string& line) const;
          std::string translateOutputSyntax(const std::string& line) const;
          PythonCode extractSpecialBlocks(const std::string& code) const;
          //PythonCodeBlock concatenateNormalBlocks(const PythonCode& code) const;
          PythonCodeBlock concatenateAndTranslateMatlabBlocks(const PythonCode& code) const;
          std::string translateMatlabBlock(const PythonCodeBlock& code) const;
        private:
          ModuleIdGetter moduleId_;
          const Dataflow::Networks::ModuleStateHandle state_;
          std::vector<AlgorithmParameterName> outputNamesToCheck_;
          std::vector<std::string> portIds_;
          void parsePart(PythonCode& blocks, const std::string& part) const;
        };
      }
    }
  }
}

#endif
