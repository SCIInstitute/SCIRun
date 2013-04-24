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

#include <iostream>
#include <boost/make_shared.hpp>
#include <Core/Command/CommandQueue.h>
#include <Core/Command/GlobalCommandBuilderFromCommandLine.h>
#include <Core/Utils/Exception.h>

using namespace SCIRun::Core::Commands;

  GlobalCommandBuilderFromCommandLine::GlobalCommandBuilderFromCommandLine(GlobalCommandFactoryHandle cmdFactory) : cmdFactory_(cmdFactory)
  {
    ENSURE_NOT_NULL(cmdFactory, "CommandFactory");
  }

  CommandQueueHandle GlobalCommandBuilderFromCommandLine::build(SCIRun::Core::CommandLine::ApplicationParametersHandle params)
  {
    ENSURE_NOT_NULL(params, "Application parameters");
    CommandQueueHandle q(boost::make_shared<CommandQueue>());

    if (params->help())
    {
      q->enqueue(cmdFactory_->create(PrintHelp));
      q->enqueue(cmdFactory_->create(QuitCommand));
      return q;
    }

    if (params->version())
    {
      q->enqueue(cmdFactory_->create(PrintVersion));
      q->enqueue(cmdFactory_->create(QuitCommand));
      return q;
    }

    if (!params->disableSplash())
      q->enqueue(cmdFactory_->create(ShowSplashScreen));
    
    if (!params->disableGui())
      q->enqueue(cmdFactory_->create(ShowMainWindow));
    else
      std::cout << "HEADLESS MODE" << std::endl;  //TODO obviously

    if (params->inputFile())
      q->enqueue(cmdFactory_->create(LoadNetworkFile));
    else if (params->pythonScriptFile())
      q->enqueue(cmdFactory_->create(RunPythonScript));

    if (params->executeNetwork())
      q->enqueue(cmdFactory_->create(ExecuteCurrentNetwork));
    else if (params->executeNetworkAndQuit())
    {
      q->enqueue(cmdFactory_->create(SetupQuitAfterExecute));
      q->enqueue(cmdFactory_->create(ExecuteCurrentNetwork));
    }

    return q;
  }