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

#include <Core/ICom/IComPacket.h>
#include <string.h>
 
namespace SCIRun {

IComPacket::IComPacket() :
	UsedWithLockingHandle<Mutex>("IComPacket lock"),
	ownbuffer_(false),
	databuffer_(0),
	buffersize_(0),
	elsize_(1),
	datasize_(0),
	packtag_(0),
	packid_(0),
	param1_(0),
	param2_(0)
{
}


IComPacket::IComPacket(int buffersize) :
	UsedWithLockingHandle<Mutex>("IComPacket lock"),
	ownbuffer_(false),
	databuffer_(0),
	buffersize_(0),
	elsize_(1),
	datasize_(0),
	packtag_(0),
	packid_(0),
	param1_(0),
	param2_(0)
{
	newbuffer(buffersize);
}
		
IComPacket::~IComPacket()
{
	deletebuffer();
}


void IComPacket::setbuffer(void *buffer,int buffersize,bool own)
{
	ownbuffer_ = own;
	databuffer_ = buffer;
	buffersize_ = buffersize;
	elsize_ = 1;
	datasize_ = buffersize;
}

void IComPacket::newbuffer(int buffersize)
{
	// if the buffer is already big enough
	if (buffersize_ >= buffersize) 
	{
		// if the difference is too big, delete old buffer
		// and create a new one. This will make sure that packets that
		// are being sent regularly will not need any need memory management
		// hopefully this helps in keeping memory defragmented
		if (2*buffersize >= buffersize_) return;
	}
	if(databuffer_) deletebuffer();
	char *data = new char[buffersize];
	databuffer_ = static_cast<void *>(data);
	ownbuffer_ = true;
	buffersize_ = buffersize;
}

bool	IComPacket::detachbuffer(void* &buffer, int &size)
{
	if (ownbuffer_ == false) return(false);
	buffer = databuffer_;
	size = buffersize_;
	buffersize_ = 0;
	databuffer_ = 0;
	ownbuffer_ = false;
	return(true);
}

void	IComPacket::deletebuffer()
{
	if (ownbuffer_ == false)
	{
		databuffer_ = 0;
		buffersize_ = 0;
		ownbuffer_ = false;
		return;
	}
	delete[] static_cast<char*>(databuffer_);
	databuffer_ = 0;
	buffersize_ = 0;
	ownbuffer_ = false;
}

	
	// Get/Set header information
	// elsize = element size (1,2,4,8, or 16)
	// datasize = number of elements of size elsize in the data
	// tag = number assigned by user
	// id  = number for tracking different packages (user assigned)
	


std::string		IComPacket::getstring()
{
	if (getelsize() != 1) throw not_a_string_packet();
	std::string str(getdatasize(),'\0');
	char *ptr = static_cast<char *>(getbuffer());
	int size = getdatasize();
	if (size > buffersize()) size = buffersize();
	for (int p=0; p <size; p++) str[p] = ptr[p];
	return(str);
}


std::vector<std::string>	IComPacket::getvector()
{
	std::string str = getstring();
	size_t numnewlines = 1;
	for (size_t k = 0;k < str.size(); k++) if (str[k] == '\n') numnewlines++;
	std::vector<std::string> vec(numnewlines);

	size_t strnum = 0;
	size_t start = 0;
	for (size_t k = 0;k < (str.size()); k++) 
	{
		if ((k == (str.size()-1))&&(str[k] != '\n'))
		{
			vec[strnum++] = str.substr(start);
        }
		if (str[k] == '\n')
		{
			vec[strnum++] = str.substr(start,k);  
            start = k+1;
		}	
	}
	return(vec);
}


void	IComPacket::setstring(const std::string& str)
{
	setelsize(1);
	int size = str.size();
	setdatasize(size);
	if (buffersize() < size) newbuffer(size);
	char* ptr = static_cast<char *>(getbuffer());
	for (int p=0; p < size; p++) ptr[p]= str[p];
}
	
void	IComPacket::setvector(std::vector<std::string> &vec)
{
	std::string str;
	size_t p = 0;
    for (p=0; p < (vec.size()-1); p++) str += (vec[p] + "\n");
    str += vec[p];
	setstring(str);
}


void	IComPacket::swap_bytes(void *vbuffer,int size,int elsize)
{
   char temp;
   char *buffer = static_cast<char *>(vbuffer);

   size *= elsize;

   switch(elsize)
   {
      case 0:
      case 1:
         // Do nothing. Element size is 1 byte, so there is nothing to swap
         break;
      case 2:  
		// Do a 2 bytes element byte swap. 
		for(long p=0;p<size;p+=2)
		  { temp = buffer[p]; buffer[p] = buffer[p+1]; buffer[p+1] = temp; }
		break;
      case 4:
		// Do a 4 bytes element byte swap.
		for(long p=0;p<size;p+=4)
		  { temp = buffer[p]; buffer[p] = buffer[p+3]; buffer[p+3] = temp; 
			temp = buffer[p+1]; buffer[p+1] = buffer[p+2]; buffer[p+2] = temp; }
		break;
      case 8:
		// Do a 8 bytes element byte swap.
		for(long p=0;p<size;p+=8)
		  { temp = buffer[p]; buffer[p] = buffer[p+7]; buffer[p+7] = temp; 
			temp = buffer[p+1]; buffer[p+1] = buffer[p+6]; buffer[p+6] = temp; 
			temp = buffer[p+2]; buffer[p+2] = buffer[p+5]; buffer[p+5] = temp; 
			temp = buffer[p+3]; buffer[p+3] = buffer[p+4]; buffer[p+4] = temp; }
   	    break;
      case 16:
		// Do a 16 bytes element byte swap.
		for(long p=0;p<size;p+=16)
		  { temp = buffer[p]; buffer[p] = buffer[p+15]; buffer[p+15] = temp; 
			temp = buffer[p+1]; buffer[p+1] = buffer[p+14]; buffer[p+14] = temp; 
			temp = buffer[p+2]; buffer[p+2] = buffer[p+13]; buffer[p+13] = temp; 
			temp = buffer[p+3]; buffer[p+3] = buffer[p+12]; buffer[p+12] = temp; 
   		    temp = buffer[p+4]; buffer[p+4] = buffer[p+11]; buffer[p+11] = temp; 
			temp = buffer[p+5]; buffer[p+5] = buffer[p+10]; buffer[p+10] = temp; 
			temp = buffer[p+6]; buffer[p+6] = buffer[p+9]; buffer[p+9] = temp; 
			temp = buffer[p+7]; buffer[p+7] = buffer[p+8]; buffer[p+8] = temp; }
   	    break;
      default:
        throw invalid_data_format();
   }  
}


IComPacket* IComPacket::clone()
{
	IComPacket *ptr = new IComPacket(datasize_*elsize_);
	ptr->elsize_ = elsize_;
	ptr->datasize_ = datasize_;
	ptr->packtag_ = packtag_;
	ptr->packid_ = packid_;
	ptr->param1_ = param1_;
	ptr->param2_ = param2_;	
	::memcpy(ptr->databuffer_,databuffer_,datasize_*elsize_);
	return(ptr);
}

		
} // end namespace

