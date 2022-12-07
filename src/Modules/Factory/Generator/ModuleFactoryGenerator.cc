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


#include <Modules/Factory/Generator/ModuleFactoryGenerator.h>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/foreach.hpp>

using namespace SCIRun::Modules::Factory;
using namespace Generator;

ModuleDescriptor ModuleDescriptorJsonParser::readJsonString(const std::string& json) const
{
  using boost::property_tree::ptree;
  using boost::property_tree::read_json;
  using boost::property_tree::write_json;
  ptree modProps;
  try
  {
    std::istringstream is(json);
    read_json(is, modProps);
  }
  catch (std::exception& e)
  {
    std::cerr << "ModuleDescriptorJsonParser failed, error reading JSON: " << e.what() << "\n" << json << std::endl;
    return {};
  }

  try
  {
    ModuleDescriptor moduleDesc;
    moduleDesc.name_ = modProps.get<std::string>("module.name");
    moduleDesc.namespace_ = modProps.get<std::string>("module.namespace");
    moduleDesc.status_ = modProps.get<std::string>("module.status");
    moduleDesc.description_ = modProps.get<std::string>("module.description");
    moduleDesc.header_ = modProps.get<std::string>("module.header");
    moduleDesc.algo_ = {
      modProps.get<std::string>("algorithm.name"),
      modProps.get<std::string>("algorithm.namespace"),
      modProps.get<std::string>("algorithm.header")
    };
    moduleDesc.dialog_ = {
      modProps.get<std::string>("UI.name"),
      modProps.get<std::string>("UI.header")
    };
    return moduleDesc;
  }
  catch (std::exception& e)
  {
    std::cerr << "ModuleDescriptorJsonParser read failed, missing JSON element: " << e.what() << "\n" << json << std::endl;
    return {};
  }
}

std::vector<std::string> Generator::GetListOfModuleDescriptorFiles(const std::string& path)
{
  using namespace boost::filesystem;
  std::vector<std::string> files;
  try
  {
    if (exists(path))
    {
      if (is_directory(path))
      {
        BOOST_FOREACH(const auto& file, std::make_pair(directory_iterator(path), directory_iterator()))
        {
          if (file.path().extension() == ".module")
            files.push_back(file.path().string());
        }
      }
    }
  }
  catch (const filesystem_error&)
  {
  }
  std::sort(files.begin(), files.end());
  return files;
}

ModuleDescriptor Generator::MakeDescriptorFromFile(const std::string& filename)
{
  if (!boost::filesystem::exists(filename))
    return {};
  std::ifstream in(filename);
  std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  ModuleDescriptorJsonParser parser;
  return parser.readJsonString(str);
}

ModuleDescriptorMap Generator::BuildModuleDescriptorMap(const std::vector<std::string>& descriptorFiles)
{
  ModuleDescriptorMap mdm;
  for (const auto& file : descriptorFiles)
  {
    auto desc = MakeDescriptorFromFile(file);
    mdm[desc.name_] = desc;
  }
  return mdm;
}

ModuleFactoryCodeBuilder::ModuleFactoryCodeBuilder(const ModuleDescriptorMap& descriptors) : descMap_(descriptors) {}

void ModuleFactoryCodeBuilder::start()
{
  buffer_ << "#include <Modules/Factory/ModuleDescriptionLookup.h>\n\n";
}

void ModuleFactoryCodeBuilder::addIncludes()
{
  for (const auto& desc : descMap_)
  {
    buffer_ << "#include <" << desc.second.header_ << ">\n";
  }
}

void ModuleFactoryCodeBuilder::addNamespaces()
{
  buffer_ << "\nusing namespace SCIRun::Modules::Factory;\n";
  for (const auto& desc : descMap_)
  {
    buffer_ << "using namespace SCIRun::Modules::" << desc.second.namespace_ << ";\n";
  }
}

void ModuleFactoryCodeBuilder::addDescriptionInserters()
{
  buffer_ << "\nvoid ModuleDescriptionLookup::addGeneratedModules()\n{\n";
  for (const auto& desc : descMap_)
  {
    buffer_ << "  addModuleDesc<" << desc.first << ">(\"" << desc.second.status_ << "\", \"" << desc.second.description_ << "\");\n";
  }
  buffer_ << "}\n";
}

std::string ModuleFactoryCodeBuilder::build()
{
  return buffer_.str();
}

std::string Generator::GenerateModuleCodeFileFromMap(const ModuleDescriptorMap& descriptors)
{
  ModuleFactoryCodeBuilder builder(descriptors);
  builder.start();
  builder.addIncludes();
  builder.addNamespaces();
  builder.addDescriptionInserters();
  return builder.build();
}

std::string Generator::GenerateAlgorithmCodeFileFromMap(const ModuleDescriptorMap& descriptors)
{
  AlgorithmFactoryCodeBuilder builder(descriptors);
  builder.start();
  builder.addIncludes();
  builder.addNamespaces();
  builder.addDescriptionInserters();
  return builder.build();
}

std::string Generator::GenerateModuleCodeFileFromDescriptorPath(const std::string& descriptorPath)
{
  auto files = GetListOfModuleDescriptorFiles(descriptorPath);
  auto map = BuildModuleDescriptorMap(files);
  return GenerateModuleCodeFileFromMap(map);
}

std::string Generator::GenerateAlgorithmCodeFileFromDescriptorPath(const std::string& descriptorPath)
{
  auto files = GetListOfModuleDescriptorFiles(descriptorPath);
  auto map = BuildModuleDescriptorMap(files);
  return GenerateAlgorithmCodeFileFromMap(map);
}

std::string Generator::GenerateModuleCodeFileFromSourcePath(const std::string& sourcePath)
{
  boost::filesystem::path base(sourcePath); // should be src/Modules/Factory
  auto configPath = base / "Config";
  return GenerateModuleCodeFileFromDescriptorPath(configPath.string());
}

std::string Generator::GenerateAlgorithmCodeFileFromSourcePath(const std::string& sourcePath)
{
  boost::filesystem::path base(sourcePath);
  auto configPath = base / "Config";
  return GenerateAlgorithmCodeFileFromDescriptorPath(configPath.string());
}

AlgorithmFactoryCodeBuilder::AlgorithmFactoryCodeBuilder(const ModuleDescriptorMap& descriptors) : descMap_(descriptors) {}

void AlgorithmFactoryCodeBuilder::start()
{
  buffer_ << "#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>\n#include <boost/functional/factory.hpp>\n\n";
}

void AlgorithmFactoryCodeBuilder::addIncludes()
{
  for (const auto& desc : descMap_)
  {
    const auto& header = desc.second.algo_.header_;
    if (header.find("N/A") == std::string::npos)
      buffer_ << "#include <" << header << ">\n";
  }
}

void AlgorithmFactoryCodeBuilder::addNamespaces()
{
  buffer_ << "\nusing namespace SCIRun::Core::Algorithms;\n";
  for (const auto& desc : descMap_)
  {
    const auto& ns = desc.second.algo_.namespace_;
    if (ns.find("N/A") == std::string::npos)
      buffer_ << "using namespace SCIRun::Core::Algorithms::" << ns << ";\n";
  }
}

void AlgorithmFactoryCodeBuilder::addDescriptionInserters()
{
  buffer_ << "\nvoid HardCodedAlgorithmFactory::addToMakerMapGenerated()\n{\n";
  for (const auto& desc : descMap_)
  {
    const auto& algoName = desc.second.algo_.name_;
    if (algoName.find("N/A") == std::string::npos)
      buffer_ << "  ADD_MODULE_ALGORITHM_GENERATED(" << desc.second.name_ << ", " << algoName << ");\n";
  }
  buffer_ << "}\n";
}

std::string AlgorithmFactoryCodeBuilder::build()
{
  return buffer_.str();
}

std::string Generator::GenerateDialogCodeFileFromDescriptorPath(const std::string& descriptorPath)
{
  auto files = GetListOfModuleDescriptorFiles(descriptorPath);
  auto map = BuildModuleDescriptorMap(files);
  return GenerateDialogCodeFileFromMap(map);
}

std::string Generator::GenerateDialogCodeFileFromMap(const ModuleDescriptorMap& descriptors)
{
  DialogFactoryCodeBuilder builder(descriptors);
  builder.start();
  builder.addIncludes();
  builder.addNamespaces();
  builder.addDescriptionInserters();
  return builder.build();
}

std::string Generator::GenerateDialogCodeFileFromSourcePath(const std::string& sourcePath)
{
  boost::filesystem::path base(sourcePath);
  auto configPath = base / "Config";
  return GenerateDialogCodeFileFromDescriptorPath(configPath.string());
}

DialogFactoryCodeBuilder::DialogFactoryCodeBuilder(const ModuleDescriptorMap& descriptors) : descMap_(descriptors) {}

void DialogFactoryCodeBuilder::start()
{
  buffer_ << "#include <Interface/Modules/Factory/ModuleDialogFactory.h>\n"
  "#include <boost/assign.hpp>\n"
  "#include <boost/functional/factory.hpp>\n\n";
}

void DialogFactoryCodeBuilder::addIncludes()
{
  for (const auto& desc : descMap_)
  {
    const auto& header = desc.second.dialog_.header_;
    if (header.find("N/A") == std::string::npos)
      buffer_ << "#include <" << header << ">\n";
  }
}

void DialogFactoryCodeBuilder::addNamespaces()
{
  buffer_ << "\nusing namespace SCIRun::Gui;\nusing namespace boost::assign;\n\n";
}

void DialogFactoryCodeBuilder::addDescriptionInserters()
{
  buffer_ << "void ModuleDialogFactory::addDialogsToMakerMapGenerated()\n{\n  insert(dialogMakerMap_)\n";
  for (const auto& desc : descMap_)
  {
    const auto& dialogName = desc.second.dialog_.name_;
    if (dialogName.find("N/A") == std::string::npos)
      buffer_ << "    ADD_MODULE_DIALOG(" << desc.second.name_ << ", " << dialogName << ")\n";
  }
  buffer_ << "  ;\n}\n";
}

std::string DialogFactoryCodeBuilder::build()
{
  return buffer_.str();
}
