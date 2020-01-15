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


#include <gtest/gtest.h>

/// @todo: this won't link in debug mode due to Application::Instance impl
#if NDEBUG
#include <Core/Application/Application.h>

using namespace SCIRun::Core;
using namespace SCIRun::Core::CommandLine;

TEST(ApplicationSingletonTest, CanCreateAndParseCommandLine)
{
  Application& app = Application::Instance();

  ApplicationParametersHandle appParams = app.parameters();
  EXPECT_FALSE(appParams);

  const char* argv[] = {"scirun.exe", "-e", "network.srn5"};
  int argc = sizeof(argv)/sizeof(char*);

  app.readCommandLine(argc, argv);

  appParams = app.parameters();
  ASSERT_TRUE(appParams.get() != nullptr);

  EXPECT_EQ("network.srn5", appParams->inputFiles()[0]);
  EXPECT_TRUE(appParams->executeNetwork());
}


TEST(ApplicationTest, GetUserDirectoryConfig)
{
  Application& app = Application::Instance();
  boost::filesystem::path userDir;
  ASSERT_TRUE(app.get_user_directory(userDir, true));
  std::cout << userDir << std::endl;
}

TEST(ApplicationTest, GetUserDirectoryNotConfig)
{
  Application& app = Application::Instance();
  boost::filesystem::path userDir;
  ASSERT_TRUE(app.get_user_directory(userDir, false));
  std::cout << userDir << std::endl;
}

TEST(ApplicationTest, GetConfigDirectory)
{
  Application& app = Application::Instance();
  boost::filesystem::path config;
  ASSERT_TRUE(app.get_config_directory(config));
  std::cout << config << std::endl;
}

TEST(ApplicationTest, GetUserName)
{
  Application& app = Application::Instance();
  std::string user;
  ASSERT_TRUE(app.get_user_name(user));
  std::cout << user << std::endl;
}

#endif
