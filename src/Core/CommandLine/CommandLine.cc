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
            ("version,v", "prints out version information--TODO")
            ("execute,e", "executes the given network on startup")
            ("Execute,E", "executes the given network on startup and quits when done")
            ("datadir,d", po::value<std::string>(), "scirun data directory--TODO")
            ("regression,r", "regression test a network--TODO")
            ("logfile,l", po::value<std::string>(), "add output messages to a logfile--TODO")
            ("interactive,i", "interactive mode--TODO")
            ("headless,x", "disable GUI--TODO")
            ("input-file", po::value<std::string>(), "SCIRun Network Input File")
            ;
          
          positional_.add("input-file", -1);
        }

        po::variables_map parse(int argc, const char* argv[])
        {
          po::variables_map vm;
          po::store(po::command_line_parser(argc, argv).
            options(desc_).positional(positional_).run(), vm);
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
      bool help,
      bool version,
      bool executeNetwork,
      bool executeNetworkAndQuit,
      bool disableGui) 
      : inputFile_(inputFile), help_(help), version_(version), executeNetwork_(executeNetwork), executeNetworkAndQuit_(executeNetworkAndQuit), disableGui_(disableGui) {}

    virtual boost::optional<std::string> inputFile() const
    {
      return inputFile_;
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
  private:
    boost::optional<std::string> inputFile_;
    bool help_;
    bool version_;
    bool executeNetwork_;
    bool executeNetworkAndQuit_;
    bool disableGui_;
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
  auto parsed = impl_->parse(argc, argv);
  boost::optional<std::string> inputFile = parsed.count("input-file") != 0 ? parsed["input-file"].as<std::string>() : boost::optional<std::string>();
  ApplicationParametersHandle aph(new ApplicationParametersImpl
    (
      inputFile,
      parsed.count("help") != 0,
      parsed.count("version") != 0,
      parsed.count("execute") != 0,
      parsed.count("Execute") != 0,
      parsed.count("headless") != 0
    ));

  return aph;
}

ApplicationParameters::~ApplicationParameters()
{

}