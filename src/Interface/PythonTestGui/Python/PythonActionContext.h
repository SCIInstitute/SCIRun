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

#ifndef CORE_PYTHON_PYTHONACTIONCONTEXT_H
#define CORE_PYTHON_PYTHONACTIONCONTEXT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#include <boost/signals2.hpp>

// Core includes
#include <Core/Action/ActionContext.h>
#include <Core/Utils/EnumClass.h>

namespace Core
{

CORE_ENUM_CLASS
(
	PythonActionMode,

	/// Run actions in interactive mode: 
	/// actions that require resources not immediately available will fail.
	INTERACTIVE_E,	

	/// Run actions in batch mode:
	/// actions will wait for resources to become available and retry.
	BATCH_E,

	/// Run actions in replay mode:
	/// actions won't recorded into the provenance buffer.
	REPLAY_E
)

class PythonActionContext;
typedef boost::shared_ptr< PythonActionContext > PythonActionContextHandle;

class PythonActionContext : public Core::ActionContext
{
	// -- Constructor/destructor --
public:
	PythonActionContext();
	virtual ~PythonActionContext();

	// -- Reporting functions --
public:
	virtual void report_error( const std::string& error );
	virtual void report_warning( const std::string& warning );
	virtual void report_message( const std::string& message );
	virtual void report_result( const Core::ActionResultHandle& result );
	virtual void report_need_resource( Core::NotifierHandle notifier );
	virtual Core::ActionSource source() const;

public:
	Core::NotifierHandle get_resource_notifier();
	void reset_context();
	Core::ActionResultHandle get_result();
	std::string get_error_message();

private:
	friend class PythonInterpreter;
	void set_action_mode( PythonActionMode mode );

private:
	PythonActionMode action_mode_;
	std::string error_msg_;
	Core::NotifierHandle notifier_;
	Core::ActionResultHandle result_;
};

} //end namespace Core

#endif
