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


#include <Core/CommandLine/CommandLine.h>
#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string/join.hpp>

#include <iostream>

using namespace SCIRun::Core::CommandLine;
namespace po = boost::program_options;

namespace SCIRun {

namespace Core {

namespace CommandLine {

class CommandLineParserInternal
{
public:
  CommandLineParserInternal() : desc_("SCIRun 5 command line options")
  {
      desc_.add_options()
      ("help,h", "prints usage information")
      ("version,v", "prints out version information")
      ("execute,e", "executes the given network on startup")
      ("Execute,E", "executes the given network on startup and quits when done")
      ("datadir,d", po::value<std::string>(), "scirun data directory")
      ("regression,r", po::value<int>(), "regression test a network")
      //("logfile,l", po::value<std::string>(), "add output messages to a logfile--TODO")
      ("most-recent,1", "load the most recently used file")
      ("interactive,i", "interactive mode")
      ("save-images,z", "save all ViewScene images before quitting")
      ("headless,x", "disable GUI")
      ("input-file", po::value<std::vector<std::string>>(), "SCIRun Network Input File")
      ("script,s", po::value<std::string>(), "Python script--interpret and drop into embedded console")
      ("Script,S", po::value<std::string>(), "Python script--interpret and quit after one SCIRun execution pass")
      ("import", po::value<std::string>(), "Import a network from SCIRun 4.7")
      ("no_splash,0", "Turn off splash screen")
      ("verbose", "Turn on debug log information")
      //("threadMode", po::value<std::string>(), "network execution threading mode--DEVELOPER USE ONLY")
      //("reexecuteMode", po::value<std::string>(), "network reexecution mode--DEVELOPER USE ONLY")
      //("frameInitLimit", po::value<int>(), "ViewScene frame init limit--increase if renderer fails")
      ("guiExpandFactor", po::value<double>(), "Expansion factor for high resolution displays")
      ("max-cores", po::value<unsigned int>(), "Limit the number of cores used by multithreaded algorithms")
      ("list-modules", "print list of available modules")
      ;

      positional_.add("input-file", -1);
  }

  po::variables_map parse(int argc, const char* argv[])
  {
    po::variables_map vm;
    ///@details basic_command_line_parser::allow_unregistered is needed when launching SCIRun from OS X
    /// app bundles; the first argument in argv is the program path, the second is the
    /// process serial number (Carbon API ProcessSerialNumber struct), which matches
    /// -psn_<unique id> where the unique id matches[0-9_], for example -psn_0_1085705.
    ///
    /// If parsing and saving the process serial number flag is ever necessary, it may useful to look
    /// at Boost program options non-conventional syntax.
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

class DeveloperParametersImpl : public DeveloperParameters
{
public:
  DeveloperParametersImpl(
    const boost::optional<std::string>& threadMode,
    const boost::optional<std::string>& reexecuteMode,
    const boost::optional<int>& frameInitLimit,
    const boost::optional<int>& regressionTimeout,
    const boost::optional<unsigned int>& maxCores,
    const boost::optional<double>& guiExpandFactor
    ) : threadMode_(threadMode), reexecuteMode_(reexecuteMode), frameInitLimit_(frameInitLimit),
    regressionTimeout_(regressionTimeout), maxCores_(maxCores), guiExpandFactor_(guiExpandFactor)
  {}
  boost::optional<int> regressionTimeoutSeconds() const override
  {
    return regressionTimeout_;
  }
  boost::optional<std::string> threadMode() const  override
  {
    return threadMode_;
  }
  boost::optional<std::string> reexecuteMode() const override
  {
    return reexecuteMode_;
  }
  boost::optional<int> frameInitLimit() const override
  {
    return frameInitLimit_;
  }
  boost::optional<unsigned int> maxCores() const override
  {
    return maxCores_;
  }
  boost::optional<double> guiExpandFactor() const override
  {
    return guiExpandFactor_;
  }
private:
  boost::optional<std::string> threadMode_, reexecuteMode_;
  boost::optional<int> frameInitLimit_, regressionTimeout_;
  boost::optional<unsigned int> maxCores_;
  boost::optional<double> guiExpandFactor_;
};

class ApplicationParametersImpl : public ApplicationParameters
{
public:
  struct Flags
  {
    Flags(bool help,
      bool version,
      bool executeNetwork,
      bool executeNetworkAndQuit,
      bool disableGui,
      bool disableSplash,
      bool isRegressionMode,
      bool interactiveMode,
      bool saveImages,
      bool quitAfterOneScriptedExecution,
      bool loadMostRecentFile,
      bool isVerboseMode,
      bool printModules) : help_(help), version_(version), executeNetwork_(executeNetwork),
      executeNetworkAndQuit_(executeNetworkAndQuit), disableGui_(disableGui),
      disableSplash_(disableSplash), isRegressionMode_(isRegressionMode),
      interactiveMode_(interactiveMode),
      saveImages_(saveImages),
      quitAfterOneScriptedExecution_(quitAfterOneScriptedExecution),
      loadMostRecentFile_(loadMostRecentFile),
      isVerboseMode_(isVerboseMode),
      printModules_(printModules)
    {}
    bool help_, version_, executeNetwork_, executeNetworkAndQuit_,
      disableGui_, disableSplash_, isRegressionMode_, interactiveMode_,
      saveImages_, quitAfterOneScriptedExecution_,
      loadMostRecentFile_, isVerboseMode_, printModules_;
  };
  ApplicationParametersImpl(
    const std::string& entireCommandLine,
    std::vector<std::string>&& inputFiles,
    const boost::optional<boost::filesystem::path>& pythonScriptFile,
    const boost::optional<boost::filesystem::path>& dataDirectory,
    const boost::optional<std::string>& networkToImport,
    DeveloperParametersPtr devParams,
    const Flags& flags
   ) : entireCommandLine_(entireCommandLine),
    inputFiles_(inputFiles), pythonScriptFile_(pythonScriptFile), dataDirectory_(dataDirectory),
    networkToImport_(networkToImport),
    devParams_(devParams),
    flags_(flags)
  {}

  const std::vector<std::string>& inputFiles() const override
  {
    return inputFiles_;
  }

  boost::optional<boost::filesystem::path> pythonScriptFile() const override
  {
    return pythonScriptFile_;
  }

  boost::optional<boost::filesystem::path> dataDirectory() const override
  {
    return dataDirectory_;
  }

  boost::optional<std::string> importNetworkFile() const override
  {
    return networkToImport_;
  }

  bool help() const override
  {
    return flags_.help_;
  }

  bool version() const override
  {
    return flags_.version_;
  }

  bool executeNetwork() const override
  {
    return flags_.executeNetwork_;
  }

  bool executeNetworkAndQuit() const override
  {
    return flags_.executeNetworkAndQuit_;
  }

  bool disableGui() const override
  {
    return flags_.disableGui_;
  }

  bool disableSplash() const override
  {
    return flags_.disableSplash_;
  }

  bool isRegressionMode() const override
  {
    return flags_.isRegressionMode_;
  }

  bool interactiveMode() const override
  {
    return flags_.interactiveMode_;
  }

  bool saveViewSceneScreenshotsOnQuit() const override
  {
    return flags_.saveImages_;
  }

  bool quitAfterOneScriptedExecution() const override
  {
    return flags_.quitAfterOneScriptedExecution_;
  }

  bool loadMostRecentFile() const override
  {
    return flags_.loadMostRecentFile_;
  }

  DeveloperParametersPtr developerParameters() const override
  {
    return devParams_;
  }

  bool verboseMode() const override
  {
    return flags_.isVerboseMode_;
  }

  bool printModuleList() const override
  {
    return flags_.printModules_;
  }

  const std::string& entireCommandLine() const override
  {
    return entireCommandLine_;
  }

private:
  std::string entireCommandLine_;
  std::vector<std::string> inputFiles_;
  boost::optional<boost::filesystem::path> pythonScriptFile_;
  boost::optional<boost::filesystem::path> dataDirectory_;
  boost::optional<std::string> networkToImport_;
  DeveloperParametersPtr devParams_;
  Flags flags_;
};

}

CommandLineParser::CommandLineParser() : impl_(new CommandLineParserInternal)
{
}

std::string CommandLineParser::describe() const
{
  return impl_->describe();
}

namespace
{
  template <typename T>
  boost::optional<T> parseOptionalArg(const po::variables_map& parsed, const std::string& label)
  {
    return parsed.count(label) != 0 ? parsed[label].as<T>() : boost::optional<T>();
  }
}

ApplicationParametersHandle CommandLineParser::parse(int argc, const char* argv[]) const
{
  try
  {
    auto parsed = impl_->parse(argc, argv);
    std::vector<std::string> cmdline(argv, argv + argc);
    auto inputFiles = parsed.count("input-file") != 0 ? parsed["input-file"].as<std::vector<std::string>>() : std::vector<std::string>();
    auto pythonScriptFile = boost::optional<boost::filesystem::path>();
    if (parsed.count("script") != 0 && !parsed["script"].empty() && !parsed["script"].defaulted())
    {
      pythonScriptFile = boost::filesystem::path(parsed["script"].as<std::string>());
    }
    else if (parsed.count("Script") != 0 && !parsed["Script"].empty() && !parsed["Script"].defaulted())
    {
      pythonScriptFile = boost::filesystem::path(parsed["Script"].as<std::string>());
    }
    auto dataDirectory = boost::optional<boost::filesystem::path>();
    if (parsed.count("datadir") != 0 && !parsed["datadir"].empty() && !parsed["datadir"].defaulted())
    {
      dataDirectory = boost::filesystem::path(parsed["datadir"].as<std::string>());
    }
    auto importNetworkFile = boost::optional<std::string>();
    if (parsed.count("import") != 0 && !parsed["import"].empty() && !parsed["import"].defaulted())
    {
      importNetworkFile = parsed["import"].as<std::string>();
    }

    return boost::make_shared<ApplicationParametersImpl>
      (boost::algorithm::join(cmdline, " "),
      std::move(inputFiles),
      pythonScriptFile,
      dataDirectory,
      importNetworkFile,
      boost::make_shared<DeveloperParametersImpl>(
        parseOptionalArg<std::string>(parsed, "threadMode"),
        parseOptionalArg<std::string>(parsed, "reexecuteMode"),
        parseOptionalArg<int>(parsed, "frameInitLimit"),
        parseOptionalArg<int>(parsed, "regression"),
        parseOptionalArg<unsigned int>(parsed, "max-cores"),
        parseOptionalArg<double>(parsed, "guiExpandFactor")
      ),
      ApplicationParametersImpl::Flags(
        parsed.count("help") != 0,
        parsed.count("version") != 0,
        parsed.count("execute") != 0,
        parsed.count("Execute") != 0,
        parsed.count("headless") != 0,
        parsed.count("no_splash") != 0,
        parsed.count("regression") != 0,
        parsed.count("interactive") != 0,
        parsed.count("save-images") != 0,
        parsed.count("Script") != 0,
        parsed.count("most-recent") != 0,
        parsed.count("verbose") != 0,
        parsed.count("list-modules") != 0)
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
