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

#include <Core/CommandLine/CommandLine.h>
#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>

#include <iostream>

using namespace SCIRun::Core::CommandLine;
namespace po = boost::program_options;

namespace SCIRun {

namespace Core {

namespace CommandLine {

class CommandLineParserInternal
{
public:
  CommandLineParserInternal() : desc_("SCIRun5 basic options")
  {
      desc_.add_options()
      ("help,h", "prints usage information")
      ("version,v", "prints out version information")
      ("execute,e", "executes the given network on startup")
      ("Execute,E", "executes the given network on startup and quits when done")
      ("datadir,d", po::value<std::string>(), "scirun data directory--TODO")
      ("regression,r", "regression test a network")
      ("logfile,l", po::value<std::string>(), "add output messages to a logfile--TODO")
      ("interactive,i", "interactive mode--TODO")
      ("headless,x", "disable GUI (Qt still needed, for now)")
      ("input-file", po::value<std::string>(), "SCIRun Network Input File")
      ("script,s", po::value<std::string>(), "SCIRun Python Script")
      ("no_splash", "Turn off splash screen")
      ;
      
      positional_.add("input-file", -1);
  }

  po::variables_map parse(int argc, const char* argv[])
  {
    po::variables_map vm;
    // basic_command_line_parser::allow_unregistered is needed when launching SCIRun from OS X
    // app bundles; the first argument in argv is the program path, the second is the
    // process serial number (Carbon API ProcessSerialNumber struct), which matches
    // -psn_<unique id> where the unique id matches[0-9_], for example -psn_0_1085705.
    //
    // If parsing and saving the process serial number flag is ever necessary, it may useful to look
    // at Boost program options non-conventional syntax.
    po::store(po::command_line_parser(argc, argv).options(desc_).
                positional(positional_).allow_unregistered().run(),
              vm);
    po::notify(vm);
    return vm;
  }

  std::string describe() const
  {
    std::ostringstream ostr;
    ostr << desc_;
    return ostr.str();
  }
  
private:
  po::options_description desc_;
  po::positional_options_description positional_;
};

}}}

namespace
{

class ApplicationParametersImpl : public ApplicationParameters
{
public:
  ApplicationParametersImpl(
    const boost::optional<std::string>& inputFile,
    const boost::optional<boost::filesystem::path>& pythonScriptFile,
    bool help,
    bool version,
    bool executeNetwork,
    bool executeNetworkAndQuit,
    bool disableGui,
    bool disableSplash,
    bool isRegressionMode)
    : inputFile_(inputFile), pythonScriptFile_(pythonScriptFile), help_(help), version_(version), executeNetwork_(executeNetwork),
      executeNetworkAndQuit_(executeNetworkAndQuit), disableGui_(disableGui),
      disableSplash_(disableSplash), isRegressionMode_(isRegressionMode)
  {}

  virtual boost::optional<std::string> inputFile() const
  {
    return inputFile_;
  }

  virtual boost::optional<boost::filesystem::path> pythonScriptFile() const 
  {
    return pythonScriptFile_;
  }

  virtual bool help() const
  {
    return help_;
  }

  virtual bool version() const
  {
    return version_;
  }

  virtual bool executeNetwork() const
  {
    return executeNetwork_;
  }

  virtual bool executeNetworkAndQuit() const
  {
    return executeNetworkAndQuit_;
  }

  virtual bool disableGui() const
  {
    return disableGui_;
  }
  
  virtual bool disableSplash() const
  {
    return disableSplash_;
  }

  virtual bool isRegressionMode() const
  {
    return isRegressionMode_;
  }

private:
  boost::optional<std::string> inputFile_;
  boost::optional<boost::filesystem::path> pythonScriptFile_;
  bool help_;
  bool version_;
  bool executeNetwork_;
  bool executeNetworkAndQuit_;
  bool disableGui_;
  bool disableSplash_;
  bool isRegressionMode_;
};

}

CommandLineParser::CommandLineParser() : impl_(new CommandLineParserInternal)
{
}

std::string CommandLineParser::describe() const
{
  return impl_->describe();
}

ApplicationParametersHandle CommandLineParser::parse(int argc, const char* argv[])
{
  try
  {
    auto parsed = impl_->parse(argc, argv);
    auto inputFile = parsed.count("input-file") != 0 ? parsed["input-file"].as<std::string>() : boost::optional<std::string>();
    auto pythonScriptFile = boost::optional<boost::filesystem::path>();
    if (parsed.count("script") != 0 && !parsed["script"].empty() && !parsed["script"].defaulted())
    {
      pythonScriptFile = boost::filesystem::path(parsed["script"].as<std::string>());
    }
    return boost::make_shared<ApplicationParametersImpl>
      (
      inputFile,
      pythonScriptFile,
      parsed.count("help") != 0,
      parsed.count("version") != 0,
      parsed.count("execute") != 0,
      parsed.count("Execute") != 0,
      parsed.count("headless") != 0,
      parsed.count("no_splash") != 0,
      parsed.count("regression") != 0
      );
  }
  catch (std::exception& e)
  {
    std::cerr << "Failed to parse command line: " << e.what() << std::endl;
    exit(7);
  }
}

ApplicationParameters::~ApplicationParameters()
{

}