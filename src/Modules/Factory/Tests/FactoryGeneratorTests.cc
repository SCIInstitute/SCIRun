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

#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <sstream>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

using namespace SCIRun;
using namespace Testing;
using namespace Modules::Factory;
using namespace Dataflow::Networks;
using namespace ReplacementImpl;
using namespace Dataflow::Engine;
using namespace Core::Algorithms;


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
    std::string json = buf.str(); // {"foo":"bar"}
    std::cout << json << std::endl;

    // Read json.
    /*ptree pt2;
    std::istringstream is(json);
    read_json(is, pt2);
    std::string foo = pt2.get<std::string>("foo");
*/



    if (i > 3)
      break;
  }


  FAIL() << "foo";
}

TEST(FactoryGeneratorTests, ReadFullModuleJsonDescription)
{
  std::string moduleJson =
    "{"
    "\"module\": {"
    "\"name\": \"CreateLatVol\","
    "\"namespace\" : \"Fields\","
    "\"status\" : \"Ported module\","
    "\"description\" : \"Creates Lattice Volumes\","
    "\"header\" : \"Modules/Legacy/Fields/CreateLatVol.h\""
    "}"
    "\"algorithm\": {"
    "\"name\": \"CreateLatVolAlgo\","
    "\"namespace\" : \"Fields\","
    "\"header\" : \"Core/Algorithms/Legacy/Fields/CreateLatVolAlgo.h\""
    "}"
    "\"UI\" : {"
    "\"name\": \"CreateLatVolDialog\","
    "\"header\" : \"Interface/Modules/Fields/CreateLatVolDialog.h\""
    "}"
    "}";

}
