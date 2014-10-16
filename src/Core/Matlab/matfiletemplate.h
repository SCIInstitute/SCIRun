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


/*
 * FILE: matfiletemplate.h
 * AUTH: Jeroen G Stinstra
 * DATE: 21 FEC2004
 */
 
#ifndef MATLABIO_MATFILETEMPLATE_H
#define MATLABIO_MATFILETEMPLATE_H 1

/*
 * This header file contains templates for casting and converting matfile
 * datatypes into C++ data types. As there are many different possible data
 * type conversion possible, these functions are organized as templates. 
 * 
 * For conveniece there is function writing data in a buffer (C-style)
 * For smaller datasets the vector container can be used (inefficient for 
 * pieces of data). And for scalar data there is direc access.
 */

// template functions

namespace MatlabIO {




template<class T> void matfiledata::getandcast(T *dataptr,int dsize)
{
    // This function copies and casts the data in the matfilebuffer into
    // a new buffer specified by dataptr (address of this new buffer) with
    // size size (number of elements in this buffer)
    
    if (databuffer() == 0) return;
    if (dataptr  == 0) return;
	if (dsize == 0) return;
	if (size() == 0) return;
    if (dsize > size()) dsize = size();	// limit casting and copying to amount of data we have		
	
    switch (type())
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;
	case miUINT8: 
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;
	case miUINT16: 
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;   
	case miINT32: 
	   { signed int *ptr = static_cast<signed int *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;   
	case miUINT32: 
	   { unsigned int *ptr = static_cast<unsigned int *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;		
#ifdef JGS_MATLABIO_USE_64INTS
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;   
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64 *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;	
#endif
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;   	
	case miDOUBLE: 
	   { double *ptr = static_cast<double *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;	
        default:
           throw unknown_type();
    }
}




template<class T> void matfiledata::getandcastvector(std::vector<T> &vec)
{

    // This function copies and casts the data into a vector container
   
    int dsize = size();
    vec.resize(dsize);
	
	if (databuffer() == 0) { vec.resize(0); return; }
    if (size() == 0) { vec.resize(0); return; };
			    
    switch (type())
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;
	case miUINT8: 
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;
	case miUINT16: 
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;   
	case miINT32: 
	   { signed int *ptr = static_cast<signed int *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;   
	case miUINT32: 
	   { unsigned int *ptr = static_cast<unsigned int *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;		
#ifdef JGS_MATLABIO_USE_64INTS
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;   
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64*>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;	
#endif
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;   	
	case miDOUBLE: 
	   { double *ptr = static_cast<double *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;
        default:
           throw unknown_type();           	   
    }
}


template<class T> T matfiledata::getandcastvalue(int index)
{
    // direct access to the data

    T val = 0;
    if (databuffer() == 0) throw out_of_range();
    if (index >= size()) throw out_of_range();
    
    switch (type())
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miUINT8: 
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miUINT16: 
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miINT32: 
	   { signed int *ptr = static_cast<signed int *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miUINT32: 
	   { unsigned int *ptr = static_cast<unsigned int *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
#ifdef JGS_MATLABIO_USE_64INTS	   
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64*>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
#endif
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miDOUBLE: 
	  { double *ptr = static_cast<double *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
        default:
           throw unknown_type();           
    }
    return(val);
}



// functions inserting data


template<class T> void matfiledata::putandcast(const T *dataptr,int dsize,mitype dtype)
{
    // This function copies and casts the data in the matfilebuffer into
    // a new buffer specified by dataptr (address of this new buffer) with
    // size size (number of elements in this buffer)
   
    clear(); 
    if (dataptr  == 0) return;
    
    newdatabuffer(dsize*elsize(dtype),dtype);
    
    switch (dtype)
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<signed char>(dataptr[p]); }}
	   break;
	case miUINT8: 
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<unsigned char>(dataptr[p]); }}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<signed short>(dataptr[p]); }}
	   break;
	case miUINT16: 
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<unsigned short>(dataptr[p]); }}
	   break;   
	case miINT32: 
	   { signed int *ptr = static_cast<signed int *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<signed int>(dataptr[p]); }}
	   break;   
	case miUINT32: 
	   { unsigned int *ptr = static_cast<unsigned int *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<unsigned int>(dataptr[p]); }}
	   break;
#ifdef JGS_MATLABIO_USE_64INTS	   		
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<int64>(dataptr[p]); }}
	   break;   
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64 *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<uint64>(dataptr[p]); }}
	   break;	
#endif	   
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<float>(dataptr[p]); }}
	   break;   	
	case miDOUBLE: 
	   { double *ptr = static_cast<double *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<double>(dataptr[p]); }}
	   break;	
        default:
           throw unknown_type();              
    }
}


template<class T> void matfiledata::putandcastvector(const std::vector<T> &vec,mitype type)
{
    clear();
    
    int dsize = static_cast<int>(vec.size());
	
	if (dsize == 0) return;
    newdatabuffer(dsize*elsize(type),type);
    
    switch (type)
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<signed char>(vec[p]); }}
	   break;
	case miUINT8: 
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<unsigned char>(vec[p]); }}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<signed short>(vec[p]); }}
	   break;
	case miUINT16: 
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<unsigned short>(vec[p]); }}
	   break;   
	case miINT32: 
	   { signed int *ptr = static_cast<signed int *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<signed int>(vec[p]); }}
	   break;   
	case miUINT32: 
	   { unsigned int *ptr = static_cast<unsigned int *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<unsigned int>(vec[p]); }}
	   break;	
#ifdef JGS_MATLABIO_USE_64INTS	   	
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<int64>(vec[p]); }}
	   break;   
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64 *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<uint64>(vec[p]); }}
	   break;	
#endif	   
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<float>(vec[p]); }}
	   break;   	
	case miDOUBLE: 
	   { double *ptr = static_cast<double *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<double>(vec[p]); }}
	   break;	
        default:
           throw unknown_type();              
    }
}


template<class T> void matfiledata::putandcastvalue(const T val,int index)
{
    if (index >= size()) throw out_of_range();
    
    switch (type())
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer()); ptr[index] = static_cast<signed char>(val);}
	   break;
	case miUINT8: 
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer()); ptr[index] = static_cast<unsigned char>(val);}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer()); ptr[index] = static_cast<signed short>(val);}
	   break;
	case miUINT16: 
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer()); ptr[index] = static_cast<unsigned short>(val);}
	   break;
	case miINT32: 
	   { signed int *ptr = static_cast<signed int *>(databuffer()); ptr[index] = static_cast<signed int>(val);}
	   break;
	case miUINT32: 
	   { unsigned int *ptr = static_cast<unsigned int *>(databuffer()); ptr[index] = static_cast<unsigned int>(val);}
	   break;
#ifdef JGS_MATLABIO_USE_64INTS	   
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer()); ptr[index] = static_cast<int64>(val);}
	   break;
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64 *>(databuffer()); ptr[index] = static_cast<uint64>(val);}
	   break;
#endif	   
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer()); ptr[index] = static_cast<float>(val);}
	   break;
	case miDOUBLE: 
	  { double *ptr = static_cast<double *>(databuffer()); ptr[index] = static_cast<double>(val);}
	   break;
        default:
           throw unknown_type();           
    }
}

} //end namespace

#endif
