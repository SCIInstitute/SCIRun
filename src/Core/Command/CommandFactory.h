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


#ifndef CORE_COMMAND_COMMANDFACTORY_H
#define CORE_COMMAND_COMMANDFACTORY_H

#include <boost/noncopyable.hpp>
#include <Core/Command/Command.h>
#include <Core/Command/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Commands
    {
      template <typename CommandTypeEnum>
      class CommandFactory : boost::noncopyable
      {
      public:
        virtual ~CommandFactory() {}
        virtual CommandHandle create(CommandTypeEnum type) const = 0;
      };

      class SCISHARE GlobalCommandFactory : public CommandFactory<GlobalCommands>
      {
      };

      class SCISHARE NetworkEventCommandFactory : public CommandFactory<NetworkEventCommands>
      {
      };

      class SCISHARE NullCommandFactory : public NetworkEventCommandFactory
      {
      public:
        virtual CommandHandle create(NetworkEventCommands) const override;
      };

      typedef boost::shared_ptr<GlobalCommandFactory> GlobalCommandFactoryHandle;
      typedef boost::shared_ptr<NetworkEventCommandFactory> NetworkEventCommandFactoryHandle;
    }
  }
}

#endif
