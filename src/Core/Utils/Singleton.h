/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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
#include <Core/Utils/share.h>

namespace SCIRun
{
namespace Core
{

///@class SINGLETON:
///@brief Class for defining a singleton implementation
///@details A singleton class embeds this class to handle
///	    the creation of the singleton class.

/// NOTE: This singleton model requires that the project is linked statically

template< class T >
class Singleton : public boost::noncopyable
{
public:
	T& instance()
	{
	  static T instance_;
    return instance_;
	}
};

#define CORE_SINGLETON(name)\
public:\
	friend class SCIRun::Core::Singleton<name>;\
	static name& Instance() { return instance_.instance(); }\
\
private:\
	static SCIRun::Core::Singleton<name> instance_;\

#define CORE_SINGLETON_IMPLEMENTATION(name)\
SCIRun::Core::Singleton<name> name::instance_;

}
}

#endif
