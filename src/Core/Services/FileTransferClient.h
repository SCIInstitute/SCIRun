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

#ifndef CORE_SERVICES_FILETRANSFERCLIENT_H
#define CORE_SERVICES_FILETRANSFERCLIENT_H 1

#include <Core/Services/Service.h>
#include <Core/Services/ServiceClient.h>
#include <Core/Services/FileTransfer.h>
#include <Core/SystemCall/TempFileManager.h>
#include <stdio.h>

#include <Core/Services/share.h>

namespace SCIRun {


class SCISHARE FileTransferClient : public ServiceClient
{

  public:
    FileTransferClient();
    ~FileTransferClient();
    
    // open this service. This overloaded open function will deal as well with the first packet send
    // by the service, which is an information packet. Containing location and ID of the remote home
    // directory
    bool  open(IComAddress address, std::string servicename, int session, std::string passwd);

    // Create temp directories on the local and remote site. The pattern is a name ending with
    // XXXXXX which will be replace by an uniquely generated string of numbers and letters
    bool create_remote_tempdir(std::string pattern,std::string &tempdir);
    bool create_local_tempdir(std::string pattern,std::string &tempdir);
    
    // To identify the homedirectories, SCIRun will write a file with a hopefully unique key
    // that identifies that directory uniquely. Currently this is only done by the local clock
    // and a randomnumber generator. This needs to be replaced with the UUID code, which is
    // being included into SCIRun2, but does not have OSX support and still does not function
    // all the way. Currently id's are strings that can be replaced with any other object that
    // can be written to a string such as UUIDs 
    bool get_local_homedirid(std::string& homeid);
    bool get_remote_homedirid(std::string& homeid);
    
    // Get the path to the local SCIRun temp directory and the remote one.
    // Although the directory may point to the same underlying shared drive
    // the paths may be different. The function translate_scirun_tempdir translates
    // local filenames to remote filenames.
    bool get_local_scirun_tempdir(std::string& tempdir);
    bool get_remote_scirun_tempdir(std::string& tempdir);
    bool translate_scirun_tempdir(std::string& tempdir);
    
    // These are the functions for transfering files forwards and backwards
    // between systems. These functions will do the transfer protocol.
    // They rely on the full filename (that includes the path)
    bool get_file(std::string localfilename,std::string remotefilename);
    bool put_file(std::string remotefilename,std::string localfilename);
    
    // The next set of functions are only local and are meant to simplify 
    // the task of keeping track what the remote and local directory are called
    // set_local_dir sets the local temp directories path. This should preferably
    // be an absolute path. The set_remote_dir does the same for the remote directory
    // To generate filenames on both systems, local_file() will take the filename and
    // put the local_directory in front of it and remote_file() will put the remote
    // directory path in front of the filename. 
    inline bool set_local_dir(std::string dir);
    inline bool set_remote_dir(std::string dir);
    inline std::string local_file(std::string filename);
    inline std::string remote_file(std::string filename);
    
  private:
    std::string tempdir_;
    
    std::string local_scirun_tempdir_;
    std::string remote_scirun_tempdir_;
    std::string local_homedirid_;
    std::string remote_homedirid_;
    
    std::string remote_dir_;
    std::string local_dir_;
    
    TempFileManager tfm_;

    int fileidcnt_;
    int buffersize_;
};


typedef LockingHandle<FileTransferClient> FileTransferClientHandle;

inline bool FileTransferClient::set_local_dir(std::string dir)
{
    if (dir[dir.size()-1] != '/') dir +='/';
    local_dir_ = dir;
    return(true);
}

inline bool FileTransferClient::set_remote_dir(std::string dir)
{
    if (dir[dir.size()-1] != '/') dir +='/';
    remote_dir_ = dir;
    return(true);
}
    
inline std::string FileTransferClient::local_file(std::string filename)
{
    return(local_dir_+filename);
}

inline std::string FileTransferClient::remote_file(std::string filename)
{
    return(remote_dir_+filename);
}
 

 
} // end namespace
 
#endif
