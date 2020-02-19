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


#ifndef MODULE_FACTORY_GENERATOR_H
#define MODULE_FACTORY_GENERATOR_H

#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <Modules/Factory/Generator/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Factory {

      namespace Generator
      {
        SCISHARE std::vector<std::string> GetListOfModuleDescriptorFiles(const std::string& descriptorPath);

        struct SCISHARE AlgorithmDescriptor
        {
          std::string name_, namespace_, header_;
        };

        struct SCISHARE DialogDescriptor
        {
          std::string name_, header_;
        };

        struct SCISHARE ModuleDescriptor
        {
          std::string name_, namespace_, status_, description_, header_;
          AlgorithmDescriptor algo_;
          DialogDescriptor dialog_;
        };

        class SCISHARE ModuleDescriptorJsonParser
        {
        public:
          ModuleDescriptor readJsonString(const std::string& json) const;
        };

        SCISHARE ModuleDescriptor MakeDescriptorFromFile(const std::string& filename);

        typedef std::map<std::string, ModuleDescriptor> ModuleDescriptorMap;

        SCISHARE ModuleDescriptorMap BuildModuleDescriptorMap(const std::vector<std::string>& descriptorFiles);

        class SCISHARE ModuleFactoryCodeBuilder
        {
        public:
          explicit ModuleFactoryCodeBuilder(const ModuleDescriptorMap& descriptors);
          void start();
          void addIncludes();
          void addNamespaces();
          void addDescriptionInserters();
          std::string build();
        private:
          ModuleDescriptorMap descMap_;
          std::ostringstream buffer_;
        };

        class SCISHARE AlgorithmFactoryCodeBuilder
        {
        public:
          explicit AlgorithmFactoryCodeBuilder(const ModuleDescriptorMap& descriptors);
          void start();
          void addIncludes();
          void addNamespaces();
          void addDescriptionInserters();
          std::string build();
        private:
          ModuleDescriptorMap descMap_;
          std::ostringstream buffer_;
        };

        class SCISHARE DialogFactoryCodeBuilder
        {
        public:
          explicit DialogFactoryCodeBuilder(const ModuleDescriptorMap& descriptors);
          void start();
          void addIncludes();
          void addNamespaces();
          void addDescriptionInserters();
          std::string build();
        private:
          ModuleDescriptorMap descMap_;
          std::ostringstream buffer_;
        };

        SCISHARE std::string GenerateModuleCodeFileFromMap(const ModuleDescriptorMap& descriptors);

        SCISHARE std::string GenerateModuleCodeFileFromDescriptorPath(const std::string& descriptorPath);

        SCISHARE std::string GenerateModuleCodeFileFromSourcePath(const std::string& srcPath);

        SCISHARE std::string GenerateAlgorithmCodeFileFromMap(const ModuleDescriptorMap& descriptors);
        SCISHARE std::string GenerateAlgorithmCodeFileFromDescriptorPath(const std::string& descriptorPath);
        SCISHARE std::string GenerateAlgorithmCodeFileFromSourcePath(const std::string& srcPath);

        SCISHARE std::string GenerateDialogCodeFileFromMap(const ModuleDescriptorMap& descriptors);
        SCISHARE std::string GenerateDialogCodeFileFromDescriptorPath(const std::string& descriptorPath);
        SCISHARE std::string GenerateDialogCodeFileFromSourcePath(const std::string& srcPath);
      }
    }
  }
}

#endif
