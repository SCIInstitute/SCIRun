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

#ifndef CORE_UTILS_LOCKABLE_H
#define CORE_UTILS_LOCKABLE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/noncopyable.hpp>

namespace Core
{

class Lockable : public boost::noncopyable
{
public:
	typedef boost::mutex mutex_type;
	typedef boost::unique_lock< mutex_type > lock_type;

	Lockable() {}
	~Lockable() {}

	mutex_type& get_mutex() const
	{
		return this->mutex_;
	}

private:
	mutable mutex_type mutex_;
};

class RecursiveLockable : public boost::noncopyable
{
public:
	typedef boost::recursive_mutex mutex_type;
	typedef boost::unique_lock< mutex_type > lock_type;

	RecursiveLockable() {}
	~RecursiveLockable() {}

	mutex_type& get_mutex() const
	{
		return this->mutex_;
	}

private:
	mutable mutex_type mutex_;
};

class SharedLockable : public boost::noncopyable
{
public:
	typedef boost::shared_mutex mutex_type;
	typedef boost::unique_lock< mutex_type > lock_type;
	typedef boost::shared_lock< mutex_type > shared_lock_type;
	typedef boost::upgrade_lock< mutex_type > upgrade_lock_type;
	typedef boost::upgrade_to_unique_lock< mutex_type > upgrade_to_unique_lock_type;

	SharedLockable() {}
	~SharedLockable() {}

	mutex_type& get_mutex() const
	{
		return this->mutex_;
	}

private:
	mutable mutex_type mutex_;
};

} // end namespace Core

#endif