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


#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Modules/Factory/Generator/ModuleFactoryGenerator.h>
#include <sstream>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <Testing/Utils/SCIRunUnitTests.h>

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

using namespace SCIRun;
using namespace Modules::Factory;
using namespace Generator;
using namespace TestUtils;
using namespace ::testing;


TEST(FactoryGeneratorTests, GenerateSomeJsonFromModuleProperties)
{
  HardCodedModuleFactory factory;

  auto descMap = factory.getDirectModuleDescriptionLookupMap();

  int i = 0;
  for (const auto& mod : descMap)
  {
    ++i;
    std::cout << mod.first << " -> " << mod.second << std::endl;

    // Write json.
    ptree pt;
    pt.put("module.name", mod.first.module_name_);
    pt.put("module.namespace", "<ns>");
    pt.put("module.status", mod.second.moduleStatus_);
    pt.put("module.description", mod.second.moduleInfo_);
    std::ostringstream buf;
    write_json(buf, pt, false);
    auto json = buf.str(); // {"foo":"bar"}
    std::cout << json << std::endl;

    if (i > 3)
      break;
  }
}

namespace
{
  std::string moduleJson =
    "{\n"
    "\t\"module\": {\n"
    "\t\t\"name\": \"CreateLatVol\",\n"
    "\t\t\"namespace\" : \"Fields\",\n"
    "\t\t\"status\" : \"Ported module\",\n"
    "\t\t\"description\" : \"Creates Lattice Volumes\",\n"
    "\t\t\"header\" : \"Modules/Legacy/Fields/CreateLatVol.h\"\n"
    "\t},\n"
    "\t\"algorithm\": {\n"
    "\t\t\"name\": \"CreateLatVolAlgo\",\n"
    "\t\t\"namespace\" : \"Fields\",\n"
    "\t\t\"header\" : \"Core/Algorithms/Legacy/Fields/CreateLatVolAlgo.h\"\n"
    "\t},\n"
    "\t\"UI\" : {\n"
    "\t\t\"name\": \"CreateLatVolDialog\",\n"
    "\t\t\"header\" : \"Interface/Modules/Fields/CreateLatVolDialog.h\"\n"
    "\t}\n"
    "}\n";
}

TEST(FactoryGeneratorTests, ReadFullModuleJsonDescription)
{
  std::cout << moduleJson << std::endl;

  ptree modProps;
  std::istringstream is(moduleJson);
  read_json(is, modProps);

  EXPECT_EQ("CreateLatVol", modProps.get<std::string>("module.name"));
  EXPECT_EQ("Fields", modProps.get<std::string>("module.namespace"));
  EXPECT_EQ("Ported module", modProps.get<std::string>("module.status"));
  EXPECT_EQ("Creates Lattice Volumes", modProps.get<std::string>("module.description"));
  EXPECT_EQ("Modules/Legacy/Fields/CreateLatVol.h", modProps.get<std::string>("module.header"));

  EXPECT_EQ("CreateLatVolAlgo", modProps.get<std::string>("algorithm.name"));
  EXPECT_EQ("Fields", modProps.get<std::string>("algorithm.namespace"));
  EXPECT_EQ("Core/Algorithms/Legacy/Fields/CreateLatVolAlgo.h", modProps.get<std::string>("algorithm.header"));

  EXPECT_EQ("CreateLatVolDialog", modProps.get<std::string>("UI.name"));
  EXPECT_EQ("Interface/Modules/Fields/CreateLatVolDialog.h", modProps.get<std::string>("UI.header"));

  EXPECT_THROW(modProps.get<std::string>("UI.namespace"), std::exception);
}

TEST(FactoryGeneratorTests, ReadFullModuleJsonDescriptionUsingObject)
{
  std::cout << moduleJson << std::endl;

  ModuleDescriptorJsonParser parser;

  auto desc = parser.readJsonString(moduleJson);

  EXPECT_EQ("CreateLatVol", desc.name_);
  EXPECT_EQ("Fields", desc.namespace_);
  EXPECT_EQ("Ported module", desc.status_);
  EXPECT_EQ("Creates Lattice Volumes", desc.description_);
  EXPECT_EQ("Modules/Legacy/Fields/CreateLatVol.h", desc.header_);

  //TODO
  //EXPECT_EQ("CreateLatVolAlgo", modProps.get<std::string>("algorithm.name"));
  //EXPECT_EQ("Fields", modProps.get<std::string>("algorithm.namespace"));
  //EXPECT_EQ("Core/Algorithms/Legacy/Fields/CreateLatVolAlgo.h", modProps.get<std::string>("algorithm.header"));

  //EXPECT_EQ("CreateLatVolDialog", modProps.get<std::string>("UI.name"));
  //EXPECT_EQ("Interface/Modules/Fields/CreateLatVolDialog.h", modProps.get<std::string>("UI.header"));

  //EXPECT_THROW(modProps.get<std::string>("UI.namespace"), std::exception);
}

/*
Steps:
1. CMake flag for "build module list" is turned on
2. CMake gets path to .module descriptor files, and passes it to C++ program
3. C++ program receives path and iterates over all .module files
4. For each json.module, call read code above and construct a map of structures (worry about lookup efficiency later)
5. Loop over map of module desc structures and generate code lines
6. Output of C++ code is ModuleFactory_Generated.cc file with one generated method, to be called by existing C++ factory impl
7. Once that's working, extend to generate Algo and Dialog factory functions as well
8. Then start converting old HardCodedModuleFactory lines to new way. Write some python code for this
*/

TEST(FactoryGeneratorTests, CanReadDirectoryOfDescriptorFiles)
{
  auto path = TestResources::rootDir() / "Other" / "Factory" / "Config";

  auto files = GetListOfModuleDescriptorFiles(path.string());

  EXPECT_THAT(files, ElementsAre(
    (path / "CreateLatVol.module").string(),
    (path / "GetMeshQualityField.module").string(),
    (path / "test1.module").string(),
    (path / "test2.module").string()));
}

TEST(FactoryGeneratorTests, CanBuildModuleDescriptorFromString)
{
  ModuleDescriptorJsonParser parser;

  auto desc = parser.readJsonString(moduleJson);

  EXPECT_EQ("CreateLatVol", desc.name_);
  EXPECT_EQ("Fields", desc.namespace_);
  EXPECT_EQ("Ported module", desc.status_);
  EXPECT_EQ("Creates Lattice Volumes", desc.description_);
  EXPECT_EQ("Modules/Legacy/Fields/CreateLatVol.h", desc.header_);
}

TEST(FactoryGeneratorTests, CanBuildModuleDescriptorFromFile)
{
  auto realJson = TestResources::rootDir() / "Other" / "Factory" / "Config" / "CreateLatVol.module";

  auto desc = MakeDescriptorFromFile(realJson.string());

  EXPECT_EQ("CreateLatVol", desc.name_);
  EXPECT_EQ("Fields", desc.namespace_);
  EXPECT_EQ("Ported module", desc.status_);
  EXPECT_EQ("Creates Lattice Volumes", desc.description_);
  EXPECT_EQ("Modules/Legacy/Fields/CreateLatVol.h", desc.header_);
}

TEST(FactoryGeneratorTests, CanBuildModuleDescriptorFromFile2)
{
  auto realJson = TestResources::rootDir() / "Other" / "Factory" / "Config" / "GetMeshQualityField.module";

  auto desc = MakeDescriptorFromFile(realJson.string());

  EXPECT_EQ("GetMeshQualityField", desc.name_);
  EXPECT_EQ("Fields", desc.namespace_);
  EXPECT_EQ("Ported module", desc.status_);
  EXPECT_EQ("...", desc.description_);
  EXPECT_EQ("Modules/Legacy/Fields/GetMeshQualityField.h", desc.header_);
}

TEST(FactoryGeneratorTests, InvalidJsonProducesHelpfulError)
{
  auto realJson = TestResources::rootDir() / "Other" / "Factory" / "Config" / "Invalid" / "GetMeshQualityField.module";

  auto desc = MakeDescriptorFromFile(realJson.string());
}

TEST(FactoryGeneratorTests, InvalidJsonProducesHelpfulError2)
{
  auto realJson = TestResources::rootDir() / "Other" / "Factory" / "Config" / "Invalid" / "CreateLatVol.module";

  auto desc = MakeDescriptorFromFile(realJson.string());
}

TEST(FactoryGeneratorTests, CanGenerateMapFromFileList)
{
  auto path = TestResources::rootDir() / "Other" / "Factory" / "Config" / "Real";

  auto files = GetListOfModuleDescriptorFiles(path.string());

  auto map = BuildModuleDescriptorMap(files);

  ASSERT_EQ(1, map.size());
  auto el = *map.begin();
  EXPECT_EQ("CreateLatVol", el.first);
  auto desc = el.second;
  EXPECT_EQ("CreateLatVol", desc.name_);
  EXPECT_EQ("Fields", desc.namespace_);
  EXPECT_EQ("Ported module", desc.status_);
  EXPECT_EQ("Creates Lattice Volumes", desc.description_);
  EXPECT_EQ("Modules/Legacy/Fields/CreateLatVol.h", desc.header_);
}

////// FORMAT
/*

#include <Modules/Factory/ModuleDescriptionLookup.h>
#include <[includes...]>

using namespace SCIRun::Modules::Factory;
using namespace [namespaces...]

void ModuleDescriptionLookup::addGeneratedModules()
{
  [addModuleDesc<ModuleName>("Status", "Description")];
  ...
}

*/

TEST(FactoryGeneratorTests, CanGenerateCodeFileFromMap)
{
  auto path = TestResources::rootDir() / "Other" / "Factory" / "Config" / "Real";
  auto files = GetListOfModuleDescriptorFiles(path.string());
  auto map = BuildModuleDescriptorMap(files);

  ModuleFactoryCodeBuilder builder(map);

  builder.start();

  auto partial = builder.build();
  std::string expected = "#include <Modules/Factory/ModuleDescriptionLookup.h>\n\n";
  ASSERT_EQ(expected, partial);

  builder.addIncludes();

  partial = builder.build();
  expected =
    "#include <Modules/Factory/ModuleDescriptionLookup.h>\n\n"
    "#include <Modules/Legacy/Fields/CreateLatVol.h>\n";
  ASSERT_EQ(expected, partial);

  builder.addNamespaces();

  partial = builder.build();
  expected =
    "#include <Modules/Factory/ModuleDescriptionLookup.h>\n\n"
    "#include <Modules/Legacy/Fields/CreateLatVol.h>\n\n"
    "using namespace SCIRun::Modules::Factory;\n"
    "using namespace SCIRun::Modules::Fields;\n";
  ASSERT_EQ(expected, partial);

  builder.addDescriptionInserters();

  partial = builder.build();
  expected =
    "#include <Modules/Factory/ModuleDescriptionLookup.h>\n\n"
    "#include <Modules/Legacy/Fields/CreateLatVol.h>\n\n"
    "using namespace SCIRun::Modules::Factory;\n"
    "using namespace SCIRun::Modules::Fields;\n\n"
    "void ModuleDescriptionLookup::addGeneratedModules()\n"
    "{\n"
    "  addModuleDesc<CreateLatVol>(\"Ported module\", \"Creates Lattice Volumes\");\n"
    "}\n";
  ASSERT_EQ(expected, partial);
}

TEST(FactoryGeneratorTests, CanGenerateCodeFileFromMapWithFunction)
{
  auto path = TestResources::rootDir() / "Other" / "Factory" / "Config" / "Real";
  auto files = GetListOfModuleDescriptorFiles(path.string());
  auto map = BuildModuleDescriptorMap(files);
  auto code = GenerateModuleCodeFileFromMap(map);

  std::string expectedCode = "#include <Modules/Factory/ModuleDescriptionLookup.h>\n\n"
  "#include <Modules/Legacy/Fields/CreateLatVol.h>\n\n"
  "using namespace SCIRun::Modules::Factory;\n"
  "using namespace SCIRun::Modules::Fields;\n\n"
  "void ModuleDescriptionLookup::addGeneratedModules()\n"
  "{\n"
  "  addModuleDesc<CreateLatVol>(\"Ported module\", \"Creates Lattice Volumes\");\n"
  "}\n";
  EXPECT_EQ(expectedCode, code);
}

TEST(FactoryGeneratorTests, FullProgram)
{
  auto code = GenerateModuleCodeFileFromDescriptorPath((TestResources::rootDir() / "Other" / "Factory" / "Config" / "Real").string());

  std::string expectedCode = "#include <Modules/Factory/ModuleDescriptionLookup.h>\n\n"
  "#include <Modules/Legacy/Fields/CreateLatVol.h>\n\n"
  "using namespace SCIRun::Modules::Factory;\n"
  "using namespace SCIRun::Modules::Fields;\n\n"
  "void ModuleDescriptionLookup::addGeneratedModules()\n"
  "{\n"
  "  addModuleDesc<CreateLatVol>(\"Ported module\", \"Creates Lattice Volumes\");\n"
  "}\n";
  EXPECT_EQ(expectedCode, code);
}

TEST(AlgorithmFactoryGeneratorTests, FullProgram)
{
  auto code = GenerateAlgorithmCodeFileFromDescriptorPath((TestResources::rootDir() / "Other" / "Factory" / "Config" / "Real").string());
  std::string expectedCode = "#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>\n"
  "#include <boost/functional/factory.hpp>\n\n"
  "#include <Core/Algorithms/Legacy/Fields/CreateLatVolAlgo.h>\n\n"
  "using namespace SCIRun::Core::Algorithms;\n"
  "using namespace SCIRun::Core::Algorithms::Fields;\n\n"
  "void HardCodedAlgorithmFactory::addToMakerMapGenerated()\n"
  "{\n"
  "  ADD_MODULE_ALGORITHM_GENERATED(CreateLatVol, CreateLatVolAlgo);\n"
  "}\n";
  EXPECT_EQ(expectedCode, code);
}

TEST(DialogFactoryGeneratorTests, FullProgram)
{
  auto code = GenerateDialogCodeFileFromDescriptorPath((TestResources::rootDir() / "Other" / "Factory" / "Config" / "Real").string());
  std::string expectedCode =
  "#include <Interface/Modules/Factory/ModuleDialogFactory.h>\n"
  "#include <boost/assign.hpp>\n"
  "#include <boost/functional/factory.hpp>\n\n"
  "#include <Interface/Modules/Fields/CreateLatVolDialog.h>\n"
  "\nusing namespace SCIRun::Gui;\n"
  "using namespace boost::assign;\n\n"
  "void ModuleDialogFactory::addDialogsToMakerMapGenerated()\n"
  "{\n"
  "  insert(dialogMakerMap_)\n"
  "    ADD_MODULE_DIALOG(CreateLatVol, CreateLatVolDialog)\n"
  "  ;\n}\n";

  EXPECT_EQ(expectedCode, code);

  //FAIL() << "todo";
}
