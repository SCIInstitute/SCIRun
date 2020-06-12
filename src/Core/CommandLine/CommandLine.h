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


#ifndef CORE_COMMANDLINE_COMMANDLINESPEC_H
#define CORE_COMMANDLINE_COMMANDLINESPEC_H

#include <string>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <Core/CommandLine/share.h>

namespace SCIRun {
  namespace Core {
    namespace CommandLine {

      class DeveloperParameters;
      using DeveloperParametersPtr = boost::shared_ptr<DeveloperParameters>;

      class SCISHARE ApplicationParameters : boost::noncopyable
      {
      public:
        virtual ~ApplicationParameters();
        virtual const std::vector<std::string>& inputFiles() const = 0;
        virtual boost::optional<boost::filesystem::path> pythonScriptFile() const = 0;
        virtual boost::optional<boost::filesystem::path> dataDirectory() const = 0;
        virtual boost::optional<std::string> importNetworkFile() const = 0;
        virtual bool help() const = 0;
        virtual bool version() const = 0;
        virtual bool executeNetwork() const = 0;
        virtual bool executeNetworkAndQuit() const = 0;
        virtual bool disableGui() const = 0;
        virtual bool disableSplash() const = 0;
        virtual bool isRegressionMode() const = 0;
        virtual bool interactiveMode() const = 0;
        virtual bool saveViewSceneScreenshotsOnQuit() const = 0;
        virtual bool quitAfterOneScriptedExecution() const = 0;
        virtual bool loadMostRecentFile() const = 0;
        virtual DeveloperParametersPtr developerParameters() const = 0;
        virtual bool verboseMode() const = 0;
        virtual bool printModuleList() const = 0;
        virtual const std::string& entireCommandLine() const = 0;
      };

      class SCISHARE DeveloperParameters : boost::noncopyable
      {
      public:
        virtual ~DeveloperParameters() {}
        virtual boost::optional<int> regressionTimeoutSeconds() const = 0;
        virtual boost::optional<std::string> threadMode() const = 0;
        virtual boost::optional<std::string> reexecuteMode() const = 0;
        virtual boost::optional<int> frameInitLimit() const = 0;
        virtual boost::optional<unsigned int> maxCores() const = 0;
        virtual boost::optional<double> guiExpandFactor() const = 0;
      };

      typedef boost::shared_ptr<ApplicationParameters> ApplicationParametersHandle;

      class CommandLineParserInternal;

      class SCISHARE CommandLineParser : boost::noncopyable
      {
      public:
        CommandLineParser();
        ApplicationParametersHandle parse(int argc, const char* argv[]) const;
        std::string describe() const;
      private:
        boost::shared_ptr<CommandLineParserInternal> impl_;
      };

}}}

#endif
