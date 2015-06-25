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

#include <Core/Services/Service.h>
#include <Core/Services/SimpleService.h>
#include <Core/Services/FileTransfer.h>
#include <stdio.h>
#include <string>
#include <list>

namespace SCIRun {


// Create a class for the remote service
class SCISHARE FileTransfer : public SimpleService {

public:
  // constructor
  FileTransfer(ServiceContext &ctx);

  // executable code
  bool init_service(IComPacketHandle& packet);
  void handle_service(IComPacketHandle& packet);
  void close_service();
					
private:

  FILE*   file_;
  int     fileID_;
    
  std::string filename_;
  std::list<std::string> tempdirlist_;
    
  std::string homedirid_;
  std::string scirun_tempdir_;

  int         buffersize_;    
  TempFileManager tfm_;
};

// Mark the class for the dynamic loader
DECLARE_SERVICE_MAKER(FileTransfer)


FileTransfer::FileTransfer(ServiceContext &ctx) :
  SimpleService(ctx)
{
  file_ = 0;
  fileID_ = 0;
    
  if (ctx.parameters["buffersize"] != "")
  {
    std::istringstream iss(ctx.parameters["buffersize"]);
    iss >> buffersize_; 
    if (buffersize_ < 1000) buffersize_ = 1000;
  }
  else
  {
    buffersize_ = 30000;
  }
}

bool FileTransfer::init_service(IComPacketHandle &packet)
{

  homedirid_ = tfm_.get_homedirID();
  scirun_tempdir_ = tfm_.get_scirun_tmp_dir();
    
  std::vector<std::string> data(2);
  data[0] = homedirid_;
  data[1] = scirun_tempdir_; 
    
  putmsg("FileTransfer: successfully started");
  packet->settag(TAG_FTSUCCESS);
  packet->setparam1(buffersize_);
  packet->setvector(data);
  return(true);
}


void FileTransfer::close_service()
{
  if (file_) ::fclose(file_);
  file_ = 0;
  std::list<std::string>::iterator it;
  it = tempdirlist_.begin();
  for (;it != tempdirlist_.end(); it++)
  {
    tfm_.delete_tempdir((*it));
    // We don't need it as the object gets destroyed afterwards anyway
    // tempdirlist_.erase(it);
  }
}


// Executable piece of code
void FileTransfer::handle_service(IComPacketHandle &packet)
{

  int nextpos;
  int bytesread;
  int seekpos;
    
    
  switch (packet->gettag())
  {
  case TAG_FPUT:
           
    // open a file for writing
    if (file_ != 0)
    {
      packet->setstring("Internal communication error: another file is still open");
      packet->settag(TAG_FTERROR);
      send_packet(packet);
      return;
    }
            
    filename_ = packet->getstring();
    if (filename_ == "")
    {
      packet->setstring("No filename was supplied");
      packet->settag(TAG_FTERROR);
      send_packet(packet);
      return;
    }
            
    file_ = ::fopen(filename_.c_str(),"w");
    if (file_ == 0)
    {
      packet->setstring("Could not open file");
      packet->settag(TAG_FTERROR);
      send_packet(packet);
      return;
    }
                       
    fileID_ = packet->getid(); 
    packet->clear();
    packet->settag(TAG_FNEXTDATA);
    packet->setparam1(0);
    packet->setid(fileID_);
    send_packet(packet);
            
    return;
        
  case TAG_FDATA:
            
    if (fileID_ != packet->getid())
    {
      packet->setstring("Packet has improper ID");
      packet->settag(TAG_FTERROR);
      send_packet(packet);
                
      ::fclose(file_);
      file_ = 0;
      fileID_ = 0;
      return;
    }
        
    if (::fseek(file_,static_cast<long>(packet->getparam1()),SEEK_SET) < 0)
    {
      packet->setstring("Fseek error in writing file");
      packet->settag(TAG_FTERROR);
      send_packet(packet);

      ::fclose(file_);
      file_ = 0;
      fileID_ = 0;
      return;
    }          
            
    if (::fwrite(packet->getbuffer(),1,static_cast<size_t>(packet->getdatasize()),file_) != static_cast<size_t>(packet->getdatasize()))
    {
      packet->setstring("Could not write to file");
      packet->settag(TAG_FTERROR);
      send_packet(packet);

      ::fclose(file_);
      file_ = 0;
      fileID_ = 0;
      return;          
    }
            
    nextpos = packet->getparam1()+packet->getdatasize();
    packet->clear();
    packet->setid(fileID_);
    packet->settag(TAG_FNEXTDATA);
    packet->setparam1(nextpos);
    send_packet(packet);
            
    return;
            
  case TAG_FEND:

    if (fileID_ != packet->getid())
    {
      packet->setstring("Packet has improper ID");
      packet->settag(TAG_FTERROR);
      send_packet(packet);

      ::fclose(file_);
      file_ = 0;
      fileID_ = 0;
      return;
    }

    if (::fseek(file_,static_cast<long>(packet->getparam1()),SEEK_SET) < 0)
    {
      packet->setstring("Fseek error in writing file");
      packet->settag(TAG_FTERROR);
      send_packet(packet);

      ::fclose(file_);
      file_ = 0;
      fileID_ = 0;
      return;
    }          
            
    if (::fwrite(packet->getbuffer(),1,static_cast<size_t>(packet->getdatasize()),file_) != static_cast<size_t>(packet->getdatasize()))
    {
      packet->setstring("Could not write to file");
      packet->settag(TAG_FTERROR);
      send_packet(packet);

      ::fclose(file_);
      file_ = 0;
      fileID_ = 0;
      return;          
    }

            
    ::fclose(file_);
    file_ = 0;
    fileID_ = 0;
            
    packet->clear();
    packet->settag(TAG_FTSUCCESS);
    return;
            
  case TAG_FGET:

    // open a file for writing
    if (file_ != 0)
    {
      packet->setstring("Internal communication error: another file is still open");
      packet->settag(TAG_FTERROR);
      send_packet(packet);
      return;
    }
            
    filename_ = packet->getstring();
    if (filename_ == "")
    {
      packet->setstring("No filename was supplied");
      packet->settag(TAG_FTERROR);
      send_packet(packet);
      return;
    }
            
    file_ = ::fopen(filename_.c_str(),"r");
    if (file_ == 0)
    {
      packet->setstring("Could not open file");
      packet->settag(TAG_FTERROR);
      send_packet(packet);
      return;
    }
                       
    fileID_ = packet->getid(); 
    packet->clear();
    packet->newbuffer(buffersize_);

    bytesread = ::fread(packet->getbuffer(),1,static_cast<size_t>(buffersize_),file_);

    if (bytesread < 0)
    {
      packet->clear();
      packet->setstring("Could not read file");
      packet->settag(TAG_FTERROR);
      packet->setid(fileID_);
      send_packet(packet);
      ::fclose(file_);
      file_ = 0;
      fileID_ = 0;
      return;            
    }
            
    if (bytesread <= buffersize_)
    {
      packet->setdatasize(bytesread);
      packet->settag(TAG_FEND);
      packet->setparam1(0);                
      packet->setid(fileID_);
      send_packet(packet);
                
      ::fclose(file_);
      file_ = 0;
      fileID_ = 0;
      return;                            
    }
            
    packet->setdatasize(bytesread);
    packet->settag(TAG_FDATA);
    packet->setparam1(0);
    packet->setid(fileID_);
    send_packet(packet);
    return;
            
  case TAG_FNEXTDATA:    
        
    if (fileID_ != packet->getid())
    {
      packet->setstring("Packet has improper ID");
      packet->settag(TAG_FTERROR);
      send_packet(packet);
                
      ::fclose(file_);
      file_ = 0;
      fileID_ = 0;
      return;
    }
        
    if (::fseek(file_,static_cast<long>(packet->getparam1()),SEEK_SET) < 0)
    {
      packet->setstring("Fseek error in reading file");
      packet->settag(TAG_FTERROR);
      send_packet(packet);

      ::fclose(file_);
      file_ = 0;
      fileID_ = 0;
      return;
    }            
            
    seekpos = packet->getparam1();
    packet->clear();
    packet->newbuffer(buffersize_);
    bytesread = ::fread(packet->getbuffer(),1,static_cast<size_t>(buffersize_),file_);

    if (bytesread < 0)
    {
      packet->clear();
      packet->setstring("Could not read file");
      packet->settag(TAG_FTERROR);
      packet->setid(fileID_);
      send_packet(packet);
      ::fclose(file_);
      file_ = 0;
      fileID_ = 0;
      return;            
    }
            
    if (bytesread < buffersize_)
    {
      packet->setdatasize(bytesread);
      packet->settag(TAG_FEND);
      packet->setid(fileID_);
      packet->setparam1(seekpos);
      send_packet(packet);
                
      ::fclose(file_);
      file_ = 0;
      fileID_ = 0;
      return;                            
    }
            
    packet->setdatasize(bytesread);
    packet->settag(TAG_FDATA);
    packet->setparam1(seekpos);
    packet->setid(fileID_);
    send_packet(packet);
    break;
            
  case TAG_TDIR:
    {
      std::string pattern = packet->getstring();
      if (pattern == "")
      {
	packet->setstring("No pattern was defined");
	packet->settag(TAG_FTERROR);
	send_packet(packet);
	return;
      }
      std::string tempdir;
      if(!(tfm_.create_tempdir(pattern,tempdir)))
      {
	packet->setstring("Could not create temp directory");
	packet->settag(TAG_FTERROR);
	send_packet(packet);
	return;                
      }
      tempdirlist_.push_back(tempdir);
      packet->setstring(tempdir);
      packet->settag(TAG_TDIR);
      send_packet(packet);
    }
    return;
  case TAG_FRESET:
    if (file_) ::fclose(file_);
    file_ = 0;
    fileID_ = 0;
    return;
  }
}


}

