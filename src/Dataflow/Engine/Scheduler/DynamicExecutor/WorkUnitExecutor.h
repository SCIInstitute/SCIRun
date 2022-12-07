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


#ifndef ENGINE_SCHEDULER_DYNAMICEXECUTOR_WORKUNITEXECUTOR_H
#define ENGINE_SCHEDULER_DYNAMICEXECUTOR_WORKUNITEXECUTOR_H

#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkUnitProducerInterface.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Engine/Scheduler/share.h>

namespace SCIRun {
  namespace Dataflow {
    namespace Engine {
      namespace DynamicExecutor {

        struct SCISHARE ModuleExecutor
        {
          ModuleExecutor(Networks::ModuleHandle mod, const Networks::ExecutableLookup* lookup, ProducerInterfacePtr producer) :
            module_(mod), lookup_(lookup), producer_(producer)
          {
          }
          void run() const
          {
            auto* exec = lookup_->lookupExecutable(module_->id());
            boost::signals2::scoped_connection s(exec->connectExecuteEnds([this](double, const Networks::ModuleId&) { producer_->enqueueReadyModules(); }));
            exec->executeWithSignals();
          }

          Networks::ModuleHandle module_;
          const Networks::ExecutableLookup* lookup_;
          ProducerInterfacePtr producer_;
        };


      }}

  }}

#endif
