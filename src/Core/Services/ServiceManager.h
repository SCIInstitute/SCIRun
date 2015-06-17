/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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


// ServiceManager.h

#ifndef CORE_SERVICES_SERVICEMANAGER_H
#define CORE_SERVICES_SERVICEMANAGER_H 1

#include <Core/ICom/IComAddress.h>
#include <Core/ICom/IComSocket.h>
#include <Core/Services/ServiceBase.h>
#include <Core/Services/ServiceDB.h>
#include <Core/Services/Service.h>
#include <Core/Services/ServiceFrame.h>
#include <Core/Services/ServiceLog.h>
#include <Core/Thread/Thread.h>
#include <Core/Thread/Runnable.h>
#include <Core/Thread/Mutex.h>
#include <Core/Containers/LockingHandle.h>

#include <Core/Services/share.h>
namespace SCIRun {

class SCISHARE ServiceManager: public Runnable, public ServiceBase {
public:

  // Constructor/destructor
  ServiceManager(ServiceDBHandle db, IComAddress address,
                 ServiceLogHandle log = 0);
  virtual ~ServiceManager();

  // Run will be called by the thread environment as the entry point
  // of a new thread.
  void run();
    
private:

  // Database with all the services created by the users
  ServiceLogHandle  log_;
  ServiceDBHandle   db_;
  IComAddress       address_;
};


typedef LockingHandle<ServiceManager> ServiceManagerHandle;

}

#endif
