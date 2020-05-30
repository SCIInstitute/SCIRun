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
#include <Core/CommandLine/CommandLine.h>

using namespace SCIRun::Core::CommandLine;

TEST(ScirunCommandLineSpecTest, CanReadBasicOptions)
{
  CommandLineParser parser;

  const std::string expectedHelp =
    "SCIRun 5 command line options:\n"
    "  -h [ --help ]           prints usage information\n"
    "  -v [ --version ]        prints out version information\n"
    "  -e [ --execute ]        executes the given network on startup\n"
    "  -E [ --Execute ]        executes the given network on startup and quits when \n"
    "                          done\n"
    "  -d [ --datadir ] arg    scirun data directory\n"
    "  -r [ --regression ] arg regression test a network\n"
    "  -1 [ --most-recent ]    load the most recently used file\n"
    "  -i [ --interactive ]    interactive mode\n"
    "  -z [ --save-images ]    save all ViewScene images before quitting\n"
    "  -x [ --headless ]       disable GUI\n"
    "  --input-file arg        SCIRun Network Input File\n"
    "  -s [ --script ] arg     Python script--interpret and drop into embedded \n"
    "                          console\n"
    "  -S [ --Script ] arg     Python script--interpret and quit after one SCIRun \n"
    "                          execution pass\n"
    "  --import arg            Import a network from SCIRun 4.7\n"
    "  -0 [ --no_splash ]      Turn off splash screen\n"
    "  --verbose               Turn on debug log information\n"
    "  --guiExpandFactor arg   Expansion factor for high resolution displays\n"
    "  --max-cores arg         Limit the number of cores used by multithreaded \n"
    "                          algorithms\n"
    "  --list-modules          print list of available modules\n";

  EXPECT_EQ(expectedHelp, parser.describe());

  {
    const char* argv[] = {"scirun.exe", "--help", "net.srn5"};
    int argc = sizeof(argv)/sizeof(char*);

    auto aph = parser.parse(argc, argv);

    EXPECT_TRUE(aph->help());
    EXPECT_EQ("net.srn5", aph->inputFiles()[0]);
  }

  {
    const char* argv[] = {"scirun.exe", "-h", "net.srn5"};
    int argc = sizeof(argv)/sizeof(char*);

    auto aph = parser.parse(argc, argv);

    EXPECT_TRUE(aph->help());
    EXPECT_EQ("net.srn5", aph->inputFiles()[0]);
  }

  {
    const char* argv[] = {"scirun.exe", "--version"};
    int argc = sizeof(argv)/sizeof(char*);

    auto aph = parser.parse(argc, argv);

    EXPECT_TRUE(aph->version());
    EXPECT_FALSE(aph->help());
    EXPECT_TRUE(aph->inputFiles().empty());
  }

  {
    const char* argv[] = {"scirun.exe", "-v"};
    int argc = sizeof(argv)/sizeof(char*);

    auto aph = parser.parse(argc, argv);

    EXPECT_TRUE(aph->version());
    EXPECT_FALSE(aph->help());
    EXPECT_TRUE(aph->inputFiles().empty());
  }

  {
    const char* argv[] = {"scirun.exe", "-e", "net.srn5"};
    int argc = sizeof(argv)/sizeof(char*);

    auto aph = parser.parse(argc, argv);

    EXPECT_FALSE(aph->help());
    EXPECT_TRUE(aph->executeNetwork());
    EXPECT_EQ("net.srn5", aph->inputFiles()[0]);
  }

  {
    const char* argv[] = {"scirun.exe", "-e", "net1.srn5", "net2.srn5"};
    int argc = sizeof(argv)/sizeof(char*);

    auto aph = parser.parse(argc, argv);

    EXPECT_FALSE(aph->help());
    EXPECT_TRUE(aph->executeNetwork());
    EXPECT_EQ("net1.srn5", aph->inputFiles()[0]);
    EXPECT_EQ("net2.srn5", aph->inputFiles()[1]);
  }

  {
    const char* argv[] = {"scirun.exe", "-E", "net.srn5"};
    int argc = sizeof(argv)/sizeof(char*);

    auto aph = parser.parse(argc, argv);

    EXPECT_FALSE(aph->help());
    EXPECT_TRUE(aph->executeNetworkAndQuit());
    EXPECT_FALSE(aph->executeNetwork());
    EXPECT_EQ("net.srn5", aph->inputFiles()[0]);
  }

  // {
  //   const char* argv[] = {"scirun.exe", "--threadMode", "serial"};
  //   int argc = sizeof(argv)/sizeof(char*);
  //
  //   auto aph = parser.parse(argc, argv);
  //
  //   ASSERT_TRUE(!!aph->developerParameters()->threadMode());
  //   EXPECT_EQ("serial", *aph->developerParameters()->threadMode());
  // }

  // {
  //   const char* argv[] = {"scirun.exe", "--threadMode=serial"};
  //   int argc = sizeof(argv)/sizeof(char*);
  //
  //   auto aph = parser.parse(argc, argv);
  //
  //   ASSERT_TRUE(!!aph->developerParameters()->threadMode());
  //   EXPECT_EQ("serial", *aph->developerParameters()->threadMode());
  // }

  {
    const char* argv[] = { "scirun.exe", "-1" };
    int argc = sizeof(argv) / sizeof(char*);

    auto aph = parser.parse(argc, argv);

    ASSERT_TRUE(aph->loadMostRecentFile());
  }

   {
     const char* argv[] = { "scirun.exe", "-s", "scr1.py" };
     int argc = sizeof(argv) / sizeof(char*);

     auto aph = parser.parse(argc, argv);

     EXPECT_TRUE(!!aph->pythonScriptFile());
     EXPECT_EQ("scr1.py", *aph->pythonScriptFile());
   }

  {
    const char* argv[] = { "scirun.exe", "-S", "scr1.py" };
    int argc = sizeof(argv) / sizeof(char*);

    auto aph = parser.parse(argc, argv);

    EXPECT_TRUE(!!aph->pythonScriptFile());
    EXPECT_EQ("scr1.py", *aph->pythonScriptFile());
    EXPECT_TRUE(aph->quitAfterOneScriptedExecution());
  }

  {
    const char* argv[] = { "scirun.exe", "--import", "oldnetwork.srn" };
    int argc = sizeof(argv) / sizeof(char*);

    auto aph = parser.parse(argc, argv);

    EXPECT_TRUE(!!aph->importNetworkFile());
    EXPECT_EQ("oldnetwork.srn", *aph->importNetworkFile());
  }
}
