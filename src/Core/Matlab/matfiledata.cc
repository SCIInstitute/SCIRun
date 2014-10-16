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

// NOTE: This MatlabIO file is used in different projects as well. Please, do not
// make it depend on other scirun code. This way it is easier to maintain matlabIO 
// code among different projects. Thank you.

/*
 * FILE: matfile.cc
 * AUTH: Jeroen G Stinstra
 * DATE: 16 MAY 2005
 */
 
/*
 * The matfile class is the basic interface for reading and
 * writing .mat files (matlab files). This class currently
 * only supports the so called matlab version 5 file format
 * which is used from version 5.0 to version 6.5, currently 
 * the lastest version of matlab available. Matlab V4 files
 * should be converted using matlab into the newer file format.
 *
 * This class handles the following aspects:
 * - opening and closing .mat files
 * - handling byteswapping
 * - reading/writing the file header
 * - reading/writing the tags in the .mat file
 *
 */
 
 
#include "matfiledata.h"
#include <string.h>
 
namespace MatlabIO {

// matfiledata functions

// basic constructor
matfiledata::matfiledata()
    : m_(0), ptr_(0) 
{
  m_ = new mxdata;
  m_->dataptr_ = 0; 
  m_->owndata_ = false;
  m_->bytesize_ = 0;
  m_->type_ = miUNKNOWN;
  m_->ref_ = 1; 
}
 
matfiledata::matfiledata(matfiledata::mitype type)
    : m_(0) , ptr_(0)
{
  m_ = new mxdata;
  m_->dataptr_ = 0; 
  m_->owndata_ = false;
  m_->bytesize_ = 0;
  m_->type_ = type;
  m_->ref_ = 1; 
}			 
						 	 	 	 
matfiledata::~matfiledata()
{
  if (m_ != 0)
  {
    clearptr();
  }
}
  	
void matfiledata::clear()
{
 if (m_ == 0) 
 {
   std::cerr << "internal error in clear()\n";
   throw internal_error();
 }
 if ((m_->dataptr_ != 0)&&(m_->owndata_ == true)) delete[] static_cast<char *>(m_->dataptr_);
 m_->owndata_ = false;
 m_->dataptr_ = 0;	
 m_->bytesize_ = 0;	
 m_->type_ = miUNKNOWN;
}

void matfiledata::clearptr()
{
  if (m_ == 0) return;
  m_->ref_--;
  if (m_->ref_ == 0)
  {
    clear();
    delete m_;
  }
  m_ = 0;
  ptr_ = 0;
}

matfiledata::matfiledata(const matfiledata &mfd)
{
	m_ = 0;
	m_ = mfd.m_;
	ptr_ = mfd.ptr_;
  m_->ref_++;
}
        
matfiledata& matfiledata::operator= (const matfiledata &mfd)
{
  if (this != &mfd)
  {
    clearptr();
    m_ = mfd.m_;
    ptr_ = mfd.ptr_;
    m_->ref_++;
  }
  return *this;
}


void matfiledata::newdatabuffer(int bytesize,mitype type)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in newdatabuffer()\n";
    throw internal_error();
  }
  if (m_->dataptr_ != 0) clear();
  if (m_->type_ != miMATRIX)
  {
    if (bytesize > 0)
    {
      m_->dataptr_ = static_cast<void *>(new char[bytesize]);
      m_->bytesize_ = bytesize;
    } 
  }
  m_->type_ = type; 	
  m_->owndata_ = true;
  ptr_ = 0;   
}


matfiledata matfiledata::clone()
{
	matfiledata mfd;
	
	mfd.newdatabuffer(bytesize(),type());
	memcpy(mfd.databuffer(),databuffer(),bytesize());
	mfd.ptr_ = ptr_;
	return(mfd);
}

void *matfiledata::databuffer()
{ 
  if (m_ == 0)
  {
    std::cerr << "internal error in databuffer()\n";
    throw internal_error();
	}
  if (ptr_) return(ptr_);
	return(m_->dataptr_); 
}

void matfiledata::type(mitype type)
{ 
  if (m_ == 0) 
  {
    std::cerr << "internal error in type()\n";
    throw internal_error();
  }
  m_->type_ = type; 
}

int matfiledata::bytesize()
{ 
  if (m_ == 0) 
  {
    std::cerr << "internal error in bytesize()\n";  
    throw internal_error();
	}
  if(ptr_) return(m_->bytesize_ - static_cast<int>(static_cast<char *>(ptr_) - static_cast<char *>(m_->dataptr_)));
  return(m_->bytesize_); 
}

matfiledata::mitype matfiledata::type()
{ 
  if (m_ == 0)
  {
    std::cerr << "internal error in type()\n";
    throw internal_error();
  }
  return(m_->type_); 
}

int matfiledata::elsize(matfiledata::mitype type)
{
  int elsize = 1;
   switch (type)
   {
      case miINT8: case miUINT8: case miUTF8:
         elsize = 1; break;
      case miINT16: case miUINT16: case miUTF16:
         elsize = 2; break;
      case miINT32: case miUINT32: case miSINGLE: case miUTF32:
         elsize = 4; break;
      case miUINT64: case miINT64: case miDOUBLE:
         elsize = 8; break;
      default:
        elsize = 1; break;
   }
   return(elsize);
}

int matfiledata::elsize()
{
	if (m_ == 0)
  {
    std::cerr << "internal error in elsize()\n";
    throw internal_error();
	}
  return(elsize(m_->type_));
}   		

int matfiledata::size()
{ 
  if (m_ == 0) 
  {
    std::cerr << "internal error in size()\n";
    throw internal_error();
  }
  return(m_->bytesize_/elsize()); 
}


std::string matfiledata::getstring()
{
  std::string str;

  if (size() > 0)
	{
		str.resize(size());
		getandcast<char>(&(str[0]),size());
	}
	
	return(str);
	
}	

void matfiledata::putstring(std::string str)
{
  int dsize;
  char *ptr;
  
  dsize = static_cast<int>(str.size());
  clear();
	if (dsize > 0)
	{
		newdatabuffer(dsize,miUINT8);
		ptr = static_cast<char *>(databuffer());
		for (int p=0;p<dsize;p++) { ptr[p] = str[p];} 	
	}
	else
	{
		m_->type_ = miUINT8;
	}	
}

std::vector<std::string> matfiledata::getstringarray(int maxstrlen)
{
  char *ptr;
  int numstrings;
    
	if ((maxstrlen == 0)||(size() == 0))
	{
		std::vector<std::string> emptyvec(0);
		return(emptyvec);
	}
		  
  numstrings = bytesize()/maxstrlen;
  std::vector<std::string> vec(numstrings);
  
  ptr = static_cast<char *>(databuffer());
	int q,s;
  for (q=0, s=0; q < (numstrings*maxstrlen); q+=maxstrlen,s++)
  {
    int p,r;
    for (p=0; p<maxstrlen; p++) { if (ptr[q+p] == 0) break; }
    std::string str(p,'\0');
    for (r=0;r<p;r++) { str[r] = ptr[q+r];}
    vec[s] = str;
  }
    
  return(vec);
}

int matfiledata::putstringarray(std::vector<std::string> vec)
{
  char *ptr;
  int maxstrlen = 8;
  
  for (int p=0;p<static_cast<int>(vec.size());p++) 
  { std::string str = vec[p]; if (maxstrlen < static_cast<int>(str.size()+1)) maxstrlen = static_cast<int>(str.size()+1); }
    
	maxstrlen = (((maxstrlen-1)/8)+1)*8;
	
  clear();
	int dsize = static_cast<int>(vec.size()*maxstrlen);
  newdatabuffer(dsize,miUINT8);
  ptr = static_cast<char *>(databuffer());
  
  for (int q=0, r=0;q<dsize;q+=maxstrlen,r++) 
  {
    int p;
    std::string str = vec[r];
    for (p=0;p<static_cast<int>(str.size());p++) { ptr[p+q] = str[p]; }
    for (;p<maxstrlen;p++) {ptr[p+q] = 0;}
  }		
	
	return(maxstrlen);
}



// in case of a void just copy the data (no conversion)
void matfiledata::getdata(void *dataptr,int dbytesize)
{
  if (databuffer() == 0) return;
  if (dataptr  == 0) return;
	if (dbytesize == 0) return;
	if (size() == 0) return;
  if (dbytesize > bytesize()) dbytesize = bytesize();	// limit casting and copying to amount of data we have		
	memcpy(dataptr,databuffer(),dbytesize);
}


void matfiledata::putdata(void *dataptr,int dbytesize,mitype type)
{
	clear();
	if (dataptr == 0) return;
	
	newdatabuffer(dbytesize,type);
	if (dbytesize > bytesize()) dbytesize = bytesize();
	memcpy(databuffer(),dataptr,dbytesize);
}


// Reorder of the data, this can be used to make subsets,
// transposing the matrices etc. In the current design
// you first need to create a reordering vector and then
// apply this to the data. This way the computation of the
// reordering scheme itself is separated from the actual
// reordering in memory. It has a certain memory overhead
// on the otherhand it should be more flexible

matfiledata matfiledata::reorder(const std::vector<int> &newindices)
{
	// some general integrity checks
	
	matfiledata newbuffer;
	
	if (m_ == 0)
  {
    std::cerr << "internal error in reorder()\n";
    throw internal_error();
	}
  if (databuffer() == 0) return(newbuffer);
	if (bytesize() == 0) return(newbuffer);
	
	// generate a new buffer and calculate the new size
	// as the indices specified do not need to match the
	// number of the original data field
	
	int newbytesize;
	void *newdatabuffer;
	
	newbytesize = static_cast<int>(elsize()*newindices.size());
	newbuffer.newdatabuffer(newbytesize,type());
	newdatabuffer = newbuffer.databuffer();
	
	// get the sizes of the of the old and new datablocks in elements
	
	// int osize = size();
	int dsize = static_cast<int>(newindices.size());
	
	// check limits
	// This is overhead but will prevent serious problems as it
	// checks limits and at least creates a exception in case 
	// something went wrong
	
	//for (int p = 0; p< dsize ; p++)
	//{
	//	if ((newindices[p] < 0)||(newindices[p] >= osize)) throw out_of_range();
	//}
	
	// Copy routines for each element size
	switch(elsize())
	{
		case 1:
		{
			char *data = static_cast<char *>(databuffer());
			char *newdata = static_cast<char *>(newdatabuffer);
			for (int p = 0; p < dsize ; p++) { newdata[p] = data[newindices[p]]; }
		}
		break;
		case 2:
		{
			short *data = static_cast<short *>(databuffer());
			short *newdata = static_cast<short *>(newdatabuffer);
			for (int p = 0; p < dsize ; p++) { newdata[p] = data[newindices[p]]; }
		}
		break;
		case 4:
		{
			int32_t *data = static_cast<int32_t *>(databuffer());
			int32_t *newdata = static_cast<int32_t *>(newdatabuffer);
			for (int p = 0; p < dsize ; p++) { newdata[p] = data[newindices[p]]; }
		}
		break;
		case 8:
		{
			double *data = static_cast<double *>(databuffer());
			double *newdata = static_cast<double *>(newdatabuffer);
			for (int p = 0; p < dsize ; p++) { newdata[p] = data[newindices[p]]; }
		}
		break;
		default:
      std::cerr << "internal error in reorder()\n";
      throw internal_error();
	}
	
	return(newbuffer);
}
 
 
void matfiledata::ptrset(void *ptr)
{
	ptr_ = ptr;
}

void matfiledata::ptrclear()
{
	ptr_ = 0;
} 
 
// slightly different version using C style arguments

matfiledata matfiledata::reorder(int *newindices,int dsize)
{
	// some general integrity checks
	
	matfiledata newbuffer;
	
	if (m_ == 0)
  {
    std::cerr << "internal error in reorder()\n";
    throw internal_error();
	}
  if (databuffer() == 0) return(newbuffer);
	if (bytesize() == 0) return(newbuffer);
	
	// generate a new buffer and calculate the new size
	// as the indices specified do not need to match the
	// number of the original data field
	
	int newbytesize;
	void *newdatabuffer;
	
	newbytesize = elsize()*dsize;
	newbuffer.newdatabuffer(newbytesize,type());
	newdatabuffer = newbuffer.databuffer();
	
	// get the sizes of the of the old and new datablocks in elements
	
	int osize = size();
	
	// check limits
	// This is overhead but will prevent serious problems as it
	// checks limits and at least creates a exception in case 
	// something went wrong
	
	for (int p = 0; p< dsize ; p++)
	{
		if ((newindices[p] < 0)||(newindices[p] >= osize)) throw out_of_range();
	}
	
	// Copy routines for each element size
	switch(elsize())
	{
		case 1:
		{
			char *data = static_cast<char *>(databuffer());
			char *newdata = static_cast<char *>(newdatabuffer);
			for (int p = 0; p < dsize ; p++) { newdata[p] = data[newindices[p]]; }
		}
		break;
		case 2:
		{
			short *data = static_cast<short *>(databuffer());
			short *newdata = static_cast<short *>(newdatabuffer);
			for (int p = 0; p < dsize ; p++) { newdata[p] = data[newindices[p]]; }
		}
		break;
		case 4:
		{
			int32_t *data = static_cast<int32_t *>(databuffer());
			int32_t *newdata = static_cast<int32_t *>(newdatabuffer);
			for (int p = 0; p < dsize ; p++) { newdata[p] = data[newindices[p]]; }
		}
		break;
		case 8:
		{
			double *data = static_cast<double *>(databuffer());
			double *newdata = static_cast<double *>(newdatabuffer);
			for (int p = 0; p < dsize ; p++) { newdata[p] = data[newindices[p]]; }
		}
		break;
		default:
      std::cerr << "internal error in reorder()\n";
			throw internal_error();
	}
	
	return(newbuffer);
}

matfiledata matfiledata::castdata(matfiledata::mitype type)
{
	matfiledata newdata(type);
	newdata.newdatabuffer(newdata.elsize()*size(),type);
	switch(type)
	{
		case miUINT8: case miUTF8:
		{
			unsigned char *ptr = static_cast<unsigned char *>(newdata.databuffer());
			getandcast(ptr,newdata.bytesize());
		}
		break;
		case miINT8:
		{
			signed char *ptr = static_cast<signed char *>(newdata.databuffer());
			getandcast(ptr,newdata.bytesize());
		}
		break;
		case miUINT16: case miUTF16:
		{
			unsigned short *ptr = static_cast<unsigned short *>(newdata.databuffer());
			getandcast(ptr,newdata.bytesize());
		}
		break;
		case miINT16:
		{
			signed short *ptr = static_cast<signed short *>(newdata.databuffer());
			getandcast(ptr,newdata.bytesize());
		}
		break;	
		case miUINT32: case miUTF32:
		{
			u_int32_t *ptr = static_cast<u_int32_t *>(newdata.databuffer());
			getandcast(ptr,newdata.bytesize());
		}
		break;
		case miINT32:
		{
			int32_t *ptr = static_cast<int32_t *>(newdata.databuffer());
			getandcast(ptr,newdata.bytesize());
		}
		break;
		case miUINT64:
		{
			uint64 *ptr = static_cast<uint64 *>(newdata.databuffer());
			getandcast(ptr,newdata.bytesize());
		}
		break;
		case miINT64:
		{
			int64 *ptr = static_cast<int64 *>(newdata.databuffer());
			getandcast(ptr,newdata.bytesize());
		}
		break;
		case miSINGLE:
		{
			float *ptr = static_cast<float *>(newdata.databuffer());
			getandcast(ptr,newdata.bytesize());
		}
		break;
		case miDOUBLE:
		{
			double *ptr = static_cast<double *>(newdata.databuffer());
			getandcast(ptr,newdata.bytesize());
		}
		break;	
		
		default:
      std::cerr << "internal error in castdata()\n";
			throw internal_error();
	}
	return(newdata);
}



} //end namespace

