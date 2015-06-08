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

#include <Core/Services/FileTransferClient.h>

namespace SCIRun {


FileTransferClient::FileTransferClient() :
    fileidcnt_(1),
    buffersize_(1000000)
{
}

FileTransferClient::~FileTransferClient()
{
    if (tempdir_ != "") tfm_.delete_tempdir(tempdir_);
    tempdir_ = "";
}
    

bool  FileTransferClient::open(IComAddress address, std::string servicename, int session, std::string passwd)
{
    if(!(ServiceClient::open(address,servicename,session,passwd)))
    {
        return(false);
    }
    
    IComPacketHandle packet;
    // The next function will create the packet
    if (!(recv(packet)))
    {
        seterror("Could not receive first packet");
        close();
        return(false);
    }
    if (packet->gettag() == TAG_FTERROR)
    {
        seterror(packet->getstring());
        close();
        return(false);
    }
    if (packet->gettag() == TAG_FTSUCCESS)
    {
        buffersize_ = packet->getparam1();
        std::vector<std::string> data = packet->getvector();
        
        remote_homedirid_ = data[0];
        remote_scirun_tempdir_ = data[1];

        local_homedirid_ = tfm_.get_homedirID();
        local_scirun_tempdir_ = tfm_.get_scirun_tmp_dir();
        return(true);
    }
    seterror("File transfer service returned an unknown packet");
    close();
    return(false);
}


bool FileTransferClient::create_remote_tempdir(std::string pattern,std::string &tempdir)
{
    IComPacketHandle packet = new IComPacket;
    if (packet.get_rep() == 0)
    {
        seterror("Could not create IComPacket");
        tempdir = "";
        return(false);    
    }
    packet->settag(TAG_TDIR);
    packet->setstring(pattern);
    if(!(send(packet)))
    {
        seterror("Could not send packet to filetransfer service");
        tempdir = "";
        return(false);
    }
    if(!(recv(packet)))
    {
        seterror("Could not receive packet from filetransfer service");
        tempdir = "";
        return(false);
    }
    if (packet->gettag() == TAG_FTERROR)
    {
        seterror(packet->getstring());
        tempdir = "";
        return(false);
    }
    if (packet->gettag() == TAG_TDIR)
    {
        tempdir = packet->getstring();
        return(true);
    }
    seterror("Error: received an unknown packet");
    tempdir = "";
    return(false);
}

bool FileTransferClient::create_local_tempdir(std::string pattern,std::string &tempdir)
{
    return(tfm_.create_tempdir(pattern,tempdir));
}

   
bool FileTransferClient::get_local_homedirid(std::string& homeid)
{
    homeid = local_homedirid_;
    if (homeid == "") 
    {
        local_homedirid_ = tfm_.get_homedirID();
        homeid = local_homedirid_;
        if (homeid == "") return(false);
    }
    return(true);
}

bool FileTransferClient::get_remote_homedirid(std::string& homeid)
{
    homeid = remote_homedirid_;
    if (homeid == "") return(false);
    return(true);
}

bool FileTransferClient::get_local_scirun_tempdir(std::string& tempdir)
{
    tempdir = local_scirun_tempdir_;
    if (tempdir == "") 
    {
        local_scirun_tempdir_ = tfm_.get_scirun_tmp_dir();
        tempdir = local_scirun_tempdir_;
        if (tempdir == "") return(false);
    }
    return(true);
}

bool FileTransferClient::get_remote_scirun_tempdir(std::string& tempdir)
{
    tempdir = remote_scirun_tempdir_;
    if (tempdir == "") return(false);
    return(true);
}

bool FileTransferClient::translate_scirun_tempdir(std::string& tempdir)
{
    if (remote_scirun_tempdir_ != local_scirun_tempdir_)
    {
        size_t rt = remote_scirun_tempdir_.size();
        size_t lt = local_scirun_tempdir_.size();
        
        if (rt > lt)
        {
            if (tempdir.substr(0,rt) == remote_scirun_tempdir_)
            {
                tempdir = local_scirun_tempdir_ + tempdir.substr(rt);
                return(true);
            }
            if (tempdir.substr(0,lt) == local_scirun_tempdir_)
            {
                tempdir = remote_scirun_tempdir_ + tempdir.substr(lt);
                return(true);
            }
        }
        else
        {
            if (tempdir.substr(0,lt) == local_scirun_tempdir_)
            {
                tempdir = remote_scirun_tempdir_ + tempdir.substr(lt);
                return(true);
            }
            if (tempdir.substr(0,rt) == remote_scirun_tempdir_)
            {
                tempdir = local_scirun_tempdir_ + tempdir.substr(rt);
                return(true);
            }
        }
        return(false);
    }
    else
    {
        return(true);
    }
}
        
bool FileTransferClient::get_file(std::string remotefilename,std::string localfilename)
{
    FILE* localfile;
    int fileid = fileidcnt_++;
    
    localfile = ::fopen(localfilename.c_str(),"w");
    if (localfile == 0)
    {
        seterror("Could not open local filename");
        return(false);
    }
    
    IComPacketHandle packet = new IComPacket;
    if (packet.get_rep() == 0)
    {
        ::fclose(localfile);
        seterror("Could not create IComPacket");
        return(false);    
    }    
    
    packet->settag(TAG_FGET);
    packet->setid(fileid);
    packet->setstring(remotefilename);
    if (!(send(packet)))
    {
        ::fclose(localfile);
        seterror("Could not send packet to file transfer service");
        return(false);
    }
    bool done  = false;
    
    while (!done)
    {
        if (!(recv(packet)))
        {
            ::fclose(localfile);
            seterror("Could not receive packet from file transfer service");
            return(false);
        }
        if (packet->gettag() == TAG_FTERROR)
        {
            ::fclose(localfile);
            seterror(packet->getstring());
            return(false);
        }
        if ((packet->gettag() != TAG_FDATA)&&(packet->gettag() != TAG_FEND))
        {
            ::fclose(localfile);
            packet->clear();
            packet->settag(TAG_FRESET);
            send(packet);
            seterror("Received unknown packet");
            return(false);
        }
        
        if ( ::fseek(localfile,static_cast<long>(packet->getparam1()),SEEK_SET) < 0)
        {
            ::fclose(localfile);
            packet->clear();
            packet->settag(TAG_FRESET);
            send(packet);
            seterror("Fseek error writing local file");
            return(false);
        }
        
        if ( ::fwrite(packet->getbuffer(),1,static_cast<size_t>(packet->getdatasize()),localfile) != static_cast<size_t>(packet->getdatasize()))
        {
            ::fclose(localfile);
            packet->clear();
            packet->settag(TAG_FRESET);
            send(packet);
            seterror("Error writing local file");
            return(false);        
        }
        
        if (packet->gettag() == TAG_FEND)
        {
            done = true;
            packet->clear();
            continue;
        }
        
        int nextoffset = packet->getparam1()+packet->getdatasize();
        packet->clear();
        packet->settag(TAG_FNEXTDATA);
        packet->setid(fileid);
        packet->setparam1(nextoffset);
        
        if (!(send(packet)))
        {
            ::fclose(localfile);
            seterror("Error sending packet to file transfer service");
            return(false);
        }
    }
     
    ::fclose(localfile);
    clearerror(); 
    return(true);        
}


bool FileTransferClient::put_file(std::string localfilename,std::string remotefilename)
{
    
    FILE* localfile;
    int fileid = fileidcnt_++;
    
    localfile = ::fopen(localfilename.c_str(),"r");
    if (localfile == 0)
    {
        seterror("Could not open local filename");
        return(false);
    }
    
    IComPacketHandle packet = new IComPacket;
    if (packet.get_rep() == 0)
    {
        seterror("Could not create IComPacket");
        ::fclose(localfile);
        return(false);    
    }
    
    packet->settag(TAG_FPUT);
    packet->setid(fileid);
    packet->setstring(remotefilename);
    if (!(send(packet)))
    {
        ::fclose(localfile);
        seterror("Could not send packet to file transfer service");
        return(false);
    }
    bool done  = false;

    while (!done)
    {
        if (!(recv(packet)))
        {
            ::fclose(localfile);
            seterror("Could not receive packet from file transfer service");
            return(false);
        }
        if (packet->gettag() == TAG_FTERROR)
        {
            ::fclose(localfile);
            seterror(packet->getstring());
            return(false);
        }
        if ((packet->gettag() != TAG_FNEXTDATA)&&(packet->gettag() != TAG_FEND))
        {
            ::fclose(localfile);
            packet->clear();
            packet->settag(TAG_FRESET);
            send(packet);
            seterror("Received unknown packet");
            return(false);
        }
        
        if ( ::fseek(localfile,static_cast<long>(packet->getparam1()),SEEK_SET) < 0)
        {
            ::fclose(localfile);
            packet->clear();
            packet->settag(TAG_FRESET);
            send(packet);
            seterror("Fseek error reading local file");
            return(false);
        }
        
        int seekpos = packet->getparam1();
        int bytesread = 0;
        
        packet->clear();
        packet->newbuffer(buffersize_);
        packet->setid(fileid);
        packet->setparam1(seekpos);
        
        bytesread = ::fread(packet->getbuffer(),1,buffersize_,localfile);
        
        if (bytesread < 0)
        {
            ::fclose(localfile);
            packet->clear();
            packet->settag(TAG_FRESET);
            send(packet);
            seterror("Error reading local file");
            return(false);        
        }
        
        if (bytesread < buffersize_)
        {
            packet->setdatasize(bytesread);
            packet->settag(TAG_FEND);
            done = true;
        }

        if (bytesread == buffersize_)
        {
            packet->setdatasize(bytesread);
            packet->settag(TAG_FDATA);
        }
        
        if (!(send(packet)))
        {
            ::fclose(localfile);
            seterror("Error sending packet to file transfer service");
            return(false);
        }
    }
     
    ::fclose(localfile);
    clearerror(); 
    return(true);        
}
    
 

 
} // end namespace
 

