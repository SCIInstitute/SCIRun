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

// ServiceDB.h

#ifndef CORE_SERVICES_SERVICEDB_H
#define CORE_SERVICES_SERVICEDB_H 1


#include <Core/Services/Service.h>
#include <Core/Services/ServiceNode.h>
#include <Core/Services/ServiceBase.h>
#include <Core/Util/soloader.h>
#include <Core/ICom/IComRHostList.h>
#include <Core/Containers/LockingHandle.h>


#include <string>
#include <map>


#include <Core/Services/share.h>

namespace SCIRun {

class ServiceDB;
typedef LockingHandle<ServiceDB> ServiceDBHandle;

class SCISHARE ServiceInfo {

  public:
  std::string    packagename;    // packagename where the service is defined
  std::string    servicename;    // name of the service itself,
                                 // defined in the XML file
  std::string    classname;      // name of the class used for the
                                 // service, defined in XML file
  std::string    classpackagename; // Name of the package where the class is defined
  std::string    version;        // version number of this class,
                                 // defined in XML file
  bool      activated;      // boolean indicating whether service is
                            // activated each service needs to be
                            // activated, before it can be used
  bool      disabled;       // In case the service is fully disabled
  ServiceMaker  maker;      // dynamic loading of class

  // RCFile stuff
  std::string    rcfile;        // the name of the rcfile used to
                                // configure the services
  std::string    rcfileversion;    // versionnumber of the rcfile used
                                   // to configure the service
  
  // Internet connection features
  IComRHostList  rhosts;      // If defined in the rcfile, a hostlist
                              // of hosts that are allowed to
                              // communicated with this service
  std::string    passwd;

  // Parameters compiled out of XMLfile and RCfile
  std::map<std::string,std::string> parameters;
};


class SCISHARE ServiceDB : public ServiceBase, public UsedWithLockingHandle<Mutex>
{
  public:
  ServiceDB();
  ~ServiceDB();
  
  // Load the packages and scan whether there are services that need
  // to be registered All necessary libraries will be loaded by this
  // function and the makers will be allocated
  void    loadpackages();
  
  // Activate/deactivate all services
  void    activateall();
  void    deactivateall();
  
  // Activate/deactivate only a dedicate service
  void    activate(const std::string& name);
  void    deactivate(const std::string& name);

  // Check whether a service exists
  bool    isservice(const std::string& servicename);
  
  void    printservices();
  
  // Get all the information that is contained in the services
  // database about a certain service The ServiceManager needs this to
  // launch a specified service
  ServiceInfo*  getserviceinfo(const std::string& servicename);
  ServiceDB*    clone();

  private:

  // Some of these were literaly taken out of existing SCIRun code and
  // need some modernisation Especially in memory management and the
  // use of STL
  
  bool      findmaker(ServiceInfo* info);
  bool      parse_service_rcfile(ServiceInfo *new_service, const std::string& filename);
  void      parse_and_find_service_rcfile(ServiceInfo *new_service, const std::string& xmldir);
  
  // Associate list with all the information structures
  std::map<std::string,ServiceInfo *> servicedb_;
};

// The service database to be used by the ServiceManager

}

#endif
