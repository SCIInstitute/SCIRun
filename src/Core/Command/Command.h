/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

/// @todo Documentation Core/Command/Command.h

#ifndef CORE_COMMAND_COMMAND_H
#define CORE_COMMAND_COMMAND_H

#include <boost/shared_ptr.hpp>
#include <Core/Algorithms/Base/AlgorithmParameterList.h>
#include <Core/Command/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Commands
    {
      class SCISHARE Command
      {
      public:
        virtual ~Command();
        virtual bool execute() = 0;
      };

      class SCISHARE UndoableCommand : public Command
      {
      public:
        virtual ~UndoableCommand();
        virtual void undo() = 0;
      };

      class SCISHARE RedoableCommand : public UndoableCommand
      {
      public:
        virtual ~RedoableCommand();
        virtual void redo() = 0;
      };

      class SCISHARE ParameterizedCommand : public Algorithms::AlgorithmParameterList, public Command
      {
      };

      typedef boost::shared_ptr<ParameterizedCommand> CommandHandle;

      class SCISHARE GuiCommand : public ParameterizedCommand
      {
      };

      class SCISHARE ConsoleCommand : public ParameterizedCommand
      {
      };

      enum GlobalCommands
      {
        ShowMainWindow,
        ShowSplashScreen,
        PrintHelp,
        PrintVersion,
        PrintModules,
        LoadNetworkFile,
        SaveNetworkFile,
        RunPythonScript,
        SetupDataDirectory,
        ExecuteCurrentNetwork,
        SetupQuitAfterExecute,
        QuitCommand
      };
    }
  }
}

#endif
