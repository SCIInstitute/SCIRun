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
 *  IComPacket.h 
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */
 
#ifndef JGS_SCI_CORE_ICOM_PACKET_H
#define JGS_SCI_CORE_ICOM_PACKET_H 1

#include <Core/ICom/IComBase.h>

#include <sys/types.h>

#include <string>
#include <vector>
#include <iostream>

#include <Core/ICom/share.h>

namespace SCIRun {

class IComSocket;

class SCISHARE IComPacket : public IComBase
{
  friend class IComSocket;
public:
	IComPacket();
	explicit IComPacket(int buffersize);			// Initialise with a certain buffersize

	// cloning function
	// this actually copy the memory
	// used for the internal packet service
	IComPacket* clone() const;
	
	~IComPacket();

	// Functions for managing the buffer associated with the packet.
	//  getbuffer: Get the pointer to the internal buffer
	//			    NOTE: recving/sending/adding data, may cause the buffer to be reallocated
	//              use the pointer only in between calls to the methods of this function
	//  setbuffer: Provide a buffer for the packet, the old buffer will be deleted
	//  newbuffer: Create a new buffer for the data
	//  detachbuffer: Tell the packet to not free the buffer but give the
	//				ownership to the process calling this function. Note:
	//              after calling this function the internal bufferptr is cleared
	//				and freeing the memory is up to the process calling this function
	//  deletebuffer: Free the buffer if it was created by the packet and clear all the
	//				pointers
	
	void* getbuffer();
	void	setbuffer(void *buffer,int buffersize,bool own = false);
	void	newbuffer(int buffersize);
	bool	detachbuffer(void* &buffer, int &size);
	void	deletebuffer();
	
	int		buffersize();
	
	// Get/Set header information
	// elsize = element size (1,2,4,8, or 16)
	// datasize = number of elements of size elsize in the data
	// tag = number assigned by user
	// id  = number for tracking different packages (user assigned)
	
	int		getelsize();
	int		getdatasize();
	int		gettag();
	int		getid();
	int		getparam1();
	int		getparam2();
	
	void	setelsize(int elsize);
	void	setdatasize(int datasize);
	void	settag(int tag);
	void	setid(int pacid);
	void	setparam1(int param1);
	void	setparam2(int param2);
	
	void	clear();

	
	// THESE ARE THE RECOMMENDED FUNCTIONS FOR ADDING AND RETRIEVING DATA

	// put and retrieve a string from the databuffer
	// these functions automatically set and retrieve element size
	std::string					getstring();
	void						setstring(const std::string& str);
	
	std::vector<std::string>	getvector();
	void						setvector(std::vector<std::string> &vec);
	
	// get/set a standard array, all data will be copied and the appropriate
	// casts will be made when reading or writing the data
	template<class T> std::vector<T>	getvector();
	template<class T> void				setvector(const std::vector<T> &vec);

	// Same as above but different addressing mode, data is a user allocated buffer
	// and datasize is the number of ELEMENTS (not bytes)
	// get/set a standard array, all data wll be copied and the appropriate
	// casts will be made when reading or writing the data
	template<class T> void get(T* data,int datasize);
	template<class T> void set(T* data,int datasize);

	// In case there is only one value in the packet
	template<class T> void getvalue(T &data);
	template<class T> void setvalue(T data);

	// Function used by IComSocket
	void	swap_bytes(void *data,int datasize,int elsize);

private:
	bool	ownbuffer_;
	void*	databuffer_;
	int		buffersize_;
		
	int		elsize_;
	int		datasize_;
	int		packtag_;
	int		packid_;
	int		param1_;
	int		param2_;
		
  IComPacket(const IComPacket& p);
  IComPacket& operator=(const IComPacket &p);
};

template<class T> std::vector<T>  IComPacket::getvector()
{
	int elsize = getelsize();
	int datasize = getdatasize();
			
	if (elsize != sizeof(T)) throw invalid_data_format();
	//T* ptr = static_cast<T*>(getdatabuffer());
	T* ptr = static_cast<T*>(0); // FIXME: getdatabuffer is undefined
	if (buffersize() < elsize*datasize) datasize = (buffersize()/elsize);
	
	std::vector<T> vec(datasize);
	for (int p=0; p < datasize; p++) vec[p] = ptr[p];
	return(vec);
}

template<class T> void IComPacket::setvector(const std::vector<T> &vec)
{
	int elsize = sizeof(T);
	int datasize = vec.size();
	
	if (buffersize() < elsize*datasize) newbuffer(elsize*datasize);
	T* ptr = static_cast<T*>(getbuffer());
	for(int p =0; p < datasize; p++) ptr[p] = vec[p];

	setelsize(elsize);
	setdatasize(datasize);
}


template<class T> void IComPacket::get(T* data,int datasize)
{
	int elsize = getelsize();
	int dsize = getdatasize();
		
	if (elsize != sizeof(T)) throw invalid_data_format();
	T* ptr = static_cast<T*>(getbuffer());
	if (buffersize() < elsize*dsize) dsize = (buffersize()/elsize);
	if (datasize > dsize) datasize = dsize;
	
	for (int p=0; p < datasize; p++) data[p] = ptr[p];
}

template<class T> void IComPacket::set(T* data,int datasize)
{
	int elsize = sizeof(T);
	
	if (buffersize() < elsize*datasize) newbuffer(elsize*datasize);
	T* ptr = static_cast<T*>(getbuffer());
	for(int p =0; p < datasize; p++) ptr[p] = data[p];
	setelsize(elsize);
	setdatasize(datasize);
}

template<class T> void IComPacket::getvalue(T& data)
{
	int elsize = getelsize();
	int dsize = getdatasize();
		
	if (elsize != sizeof(T)) throw invalid_data_format();
	T* ptr = static_cast<T*>(getbuffer());
	if (buffersize() < elsize*dsize) dsize = (buffersize()/elsize);
	if (dsize > 0) data = ptr[0]; else data = 0;
}

template<class T> void IComPacket::setvalue(T data)
{
	int elsize = sizeof(T);
	datasize_ = 1;
	
	if (buffersize() < elsize*datasize_) newbuffer(elsize*datasize_);
	T* ptr = static_cast<T*>(getbuffer());
	ptr[0] = data;
	setelsize(elsize);
	setdatasize(datasize_);
}

// INLINE FUNCTIONS

inline void IComPacket::clear()
{
	deletebuffer();
	packtag_ = 0;
	packid_ = 0;
	elsize_ = 1;
	datasize_ = 0;
}

inline int	IComPacket::getelsize()
{
	return(elsize_);
}

inline int	IComPacket::getdatasize()
{
	return(datasize_);
}

inline int	IComPacket::gettag()
{
	return(packtag_);
}

inline int  IComPacket::getid()
{
	return(packid_);
}

inline int  IComPacket::getparam1()
{
	return(param1_);
}

inline int  IComPacket::getparam2()
{
	return(param2_);
}

	
inline void	IComPacket::setelsize(int elsize)
{
	elsize_ = elsize;
}

inline void	IComPacket::setdatasize(int datasize)
{
	datasize_ = datasize;
}

inline void	IComPacket::settag(int tag)
{
	packtag_ = tag;
}


inline void	IComPacket::setid(int pacid)
{
	packid_ = pacid;
}

inline void	IComPacket::setparam1(int param1)
{
	param1_ = param1;
}

inline void	IComPacket::setparam2(int param2)
{
	param2_ = param2;
}

inline void *IComPacket::getbuffer()
{
	return(databuffer_);
}

inline int IComPacket::buffersize()
{
	return(buffersize_);
}

typedef boost::shared_ptr<IComPacket> IComPacketHandle;

} // end namespace

#endif

