/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
University of Utah.

License for the specific language governing rights and limitations under
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

/// @todo Documentation Core/ConsoleApplication/ConsoleApplication.cc
///  ConsoleCommandFactory.cc ConsoleCommands.cc

#include <boost/make_shared.hpp>
#include <Core/Application/Application.h>
#include <Core/ConsoleApplication/ConsoleApplication.h>
#include <Core/ConsoleApplication/ConsoleCommandFactory.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Core::Console;
using namespace SCIRun::Core::Logging;

int ConsoleApplication::run(int argc, const char* argv[])
{
  try
  {
    Application::Instance().setCommandFactory(boost::make_shared<ConsoleGlobalCommandFactory>());
    Application::Instance().executeCommandLineRequests();
  }
  catch (std::exception& e)
  {
    Log::get() << EMERG << "Unhandled exception: " << e.what() << std::endl;
    std::cerr << "Critical error! Unhandled exception: " << e.what() << "\nExiting now." << std::endl;
    return 1;
  }
  catch (...)
  {
    Log::get() << EMERG << "Unknown unhandled exception." << std::endl;
    std::cerr << "Critical error! Unknown unhandled exception: exiting now." << std::endl;
    return 1;
  }

  Application::Instance().shutdown();
  return 0;
}
