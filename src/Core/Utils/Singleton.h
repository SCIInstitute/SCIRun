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

#ifndef CORE_UTILS_SINGLETON_H
#define CORE_UTILS_SINGLETON_H

#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>

namespace SCIRun 
{
namespace Core
{

// CLASS SINGLETON:
/// Class for defining a singleton implementation
/// A singleton class embeds this class to handle 
/// the creation of the singleton class.

/// NOTE: This singleton model requires that the project is linked statically

template< class T >
class Singleton : public boost::noncopyable
{

	// -- constructor --
public:
	Singleton() :
		initialized_( false ), instance_( 0 )
	{
	}

	// -- obtain singleton pointer --
public:
	// INSTANCE:
	/// Get the singleton pointer to the application

	T* instance()
	{
		// if no singleton was allocated, allocate it now
		if ( !this->initialized_ )
		{
			//in case multiple threads try to allocate this one at once.
			{
				boost::unique_lock< boost::mutex > lock( this->instance_mutex_ );
				// The first test was not locked and hence not thread safe
				// This one will do a thread-safe allocation of the interface
				// class
				if ( this->instance_ == 0 ) this->instance_ = new T;
			}

			{
				// Enforce memory synchronization so the singleton is initialized
				// before we set initialized to true
				boost::unique_lock< boost::mutex > lock( this->instance_mutex_ );
				this->initialized_ = true;
			}
		}
		return this->instance_;
	}

	// -- internals of the singleton --
private:
	// Mutex protecting the singleton interface
	boost::mutex instance_mutex_;
	// Initialized or not?
	bool initialized_;
	// Pointer that contains the singleton interface to this class
	T* instance_;
};

#define CORE_SINGLETON(name)\
public:\
	friend class SCIRun::Core::Singleton<name>;\
	static name* Instance() { return instance_.instance(); }\
\
private:\
	static SCIRun::Core::Singleton<name> instance_;\
	
#define CORE_SINGLETON_IMPLEMENTATION(name)\
SCIRun::Core::Singleton<name> name::instance_;

}
}

#endif
