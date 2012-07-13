/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Core/Dataflow/Network/ModuleDescriptionSerialization.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>
#include <fstream>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace SCIRun::Domain::Networks;

void save_info(const ModuleLookupInfoXML& s, const char * filename)
{
  std::ofstream ofs(filename);
  assert(ofs.good());
  boost::archive::xml_oarchive oa(ofs);
  oa << BOOST_SERIALIZATION_NVP(s);
}

void restore_info(ModuleLookupInfoXML& s, const char * filename)
{
  std::ifstream ifs(filename);
  assert(ifs.good());
  boost::archive::xml_iarchive ia(ifs);

  ia >> BOOST_SERIALIZATION_NVP(s);
}

TEST(ModuleDescriptionXMLTest, CanSerializeModuleInfo)
{
  ModuleLookupInfoXML info;
  info.module_name_ = "ComputeSVD";  
  info.category_name_ = "Math";
  info.package_name_ = "SCIRun";

  std::string filename("E:\\git\\SCIRunGUIPrototype\\src\\Samples\\");
  filename += "info.xml";

  save_info(info, filename.c_str());

  ModuleLookupInfoXML newInfo;

  restore_info(newInfo, filename.c_str());

  EXPECT_EQ(info.module_name_, newInfo.module_name_);
  EXPECT_EQ(info.category_name_, newInfo.category_name_);
  EXPECT_EQ(info.package_name_, newInfo.package_name_);
}