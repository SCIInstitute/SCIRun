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


/// @todo Documentation Core/Command/GlobalCommandBuilderFromCommandLine.cc

#include <iostream>
#include <boost/make_shared.hpp>
#include <Core/Command/CommandQueue.h>
#include <Core/Command/GlobalCommandBuilderFromCommandLine.h>
#include <Core/Utils/Exception.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun::Core::Commands;
using namespace SCIRun::Core::CommandLine;
using namespace SCIRun::Core::Algorithms;

  GlobalCommandBuilderFromCommandLine::GlobalCommandBuilderFromCommandLine(GlobalCommandFactoryHandle cmdFactory) : cmdFactory_(cmdFactory)
  {
    ENSURE_NOT_NULL(cmdFactory, "CommandFactory");
  }

  CommandQueueHandle GlobalCommandBuilderFromCommandLine::build(ApplicationParametersHandle params) const
  {
    ENSURE_NOT_NULL(params, "Application parameters");
    auto q(boost::make_shared<CommandQueue>());

    if (params->help())
    {
      q->enqueue(cmdFactory_->create(GlobalCommands::PrintHelp));
      q->enqueue(cmdFactory_->create(GlobalCommands::QuitCommand));
      return q;
    }

    if (params->printModuleList())
    {
      q->enqueue(cmdFactory_->create(GlobalCommands::PrintModules));
      q->enqueue(cmdFactory_->create(GlobalCommands::QuitCommand));
      return q;
    }

    if (params->version())
    {
      q->enqueue(cmdFactory_->create(GlobalCommands::PrintVersion));
      q->enqueue(cmdFactory_->create(GlobalCommands::QuitCommand));
      return q;
    }

    if (!params->disableSplash() && !params->disableGui())
      q->enqueue(cmdFactory_->create(GlobalCommands::ShowSplashScreen));

    if (!params->disableGui())
      q->enqueue(cmdFactory_->create(GlobalCommands::ShowMainWindow));

    if (params->dataDirectory())
      q->enqueue(cmdFactory_->create(GlobalCommands::SetupDataDirectory));

    if (params->importNetworkFile())
    {
      auto import = cmdFactory_->create(GlobalCommands::ImportNetworkFile);
      import->set(Variables::Filename, *params->importNetworkFile());

      if (params->disableGui())
        import->set(Core::Algorithms::AlgorithmParameterName("QuietMode"), true);

      q->enqueue(import);
      auto save = cmdFactory_->create(GlobalCommands::SaveNetworkFile);
      save->set(Variables::Filename, (*params->importNetworkFile()) + "_imported.srn5");
      q->enqueue(save);
    }

    if (params->pythonScriptFile())
    {
      if (params->executeNetworkAndQuit())
        q->enqueue(cmdFactory_->create(GlobalCommands::SetupQuitAfterExecute));
      q->enqueue(cmdFactory_->create(GlobalCommands::RunPythonScript));
      std::cout << "Please note all args after script file name will be passed to python and not SCIRun!" << std::endl;
    }
    else if (!params->inputFiles().empty() || params->loadMostRecentFile())
    {
      const int last = 1;
      //TODO: support multiple files loaded--need to be able to execute and wait for each before loading next. See #825
      // last = params->inputFiles().size()
      for (int i = 0; i < last; ++i)
      {
        auto load = cmdFactory_->create(GlobalCommands::LoadNetworkFile);
        load->set(Name("FileNum"), i);
        if (params->loadMostRecentFile())
          load->set(Variables::Filename, mostRecentFileCode());
        q->enqueue(load);

        if (params->executeNetwork())
          q->enqueue(cmdFactory_->create(GlobalCommands::ExecuteCurrentNetwork));
        else if (params->executeNetworkAndQuit())
        {
          if (i == last - 1)
            q->enqueue(cmdFactory_->create(GlobalCommands::SetupQuitAfterExecute));
          q->enqueue(cmdFactory_->create(GlobalCommands::ExecuteCurrentNetwork));
        }
      }
    }
    else if (params->disableGui() && !params->interactiveMode())
    {
      if (!params->importNetworkFile())
        std::cout << "No input files: run with GUI or in interactive mode (-i)" << std::endl;
      q->enqueue(cmdFactory_->create(GlobalCommands::QuitCommand));
    }

    if (params->interactiveMode())
      q->enqueue(cmdFactory_->create(GlobalCommands::InteractiveMode));

    return q;
  }
