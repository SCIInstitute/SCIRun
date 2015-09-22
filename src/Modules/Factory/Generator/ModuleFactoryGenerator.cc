/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
 University of Utah.

 License for the specific language governing rights and limitations under
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
  try
  {
    using boost::property_tree::ptree;
    using boost::property_tree::read_json;
    using boost::property_tree::write_json;
    ptree modProps;
    std::istringstream is(json);
    read_json(is, modProps);

    return{
      modProps.get<std::string>("module.name"),
      modProps.get<std::string>("module.namespace"),
      modProps.get<std::string>("module.status"),
      modProps.get<std::string>("module.description"),
      modProps.get<std::string>("module.header")
    };
  }
  catch (...)
  {
    std::cerr << "ModuleDescriptorJsonParser failed:\n" << json << std::endl;
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

std::string Generator::GenerateCodeFileFromMap(const ModuleDescriptorMap& descriptors)
{
  ModuleFactoryCodeBuilder builder(descriptors);
  builder.start();
  builder.addIncludes();
  builder.addNamespaces();
  builder.addDescriptionInserters();
  return builder.build();
}

std::string Generator::GenerateCodeFileFromDescriptorPath(const std::string& descriptorPath)
{
  auto files = GetListOfModuleDescriptorFiles(descriptorPath);
  auto map = BuildModuleDescriptorMap(files);
  return GenerateCodeFileFromMap(map);
}

std::string Generator::GenerateCodeFileFromSourcePath(const std::string& sourcePath)
{
  boost::filesystem::path base(sourcePath); // should be src/Modules/Factory
  //std::cout << "__GENERATOR__ " << base << std::endl;
  auto configPath = base / "Config";
  //std::cout << "__GENERATOR__ " << configPath << std::endl;
  auto files = GetListOfModuleDescriptorFiles(configPath.string());
  //std::cout << "__GENERATOR__ " << files.size() << std::endl;
  auto map = BuildModuleDescriptorMap(files);
  //std::cout << "__GENERATOR__ " << map.size() << std::endl;
  return GenerateCodeFileFromMap(map);
}
