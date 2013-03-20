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

#include <Core/Utils/Log.h>
#include <Core/Python/PythonActionContext.h>

namespace Core
{

PythonActionContext::PythonActionContext() :
	ActionContext(),
	action_mode_( PythonActionMode::INTERACTIVE_E )
{
}

PythonActionContext::~PythonActionContext()
{
}

void PythonActionContext::report_error( const std::string& error )
{
	this->error_msg_ = error;
}

void PythonActionContext::report_warning( const std::string& warning )
{
	this->error_msg_ = warning;
}

void PythonActionContext::report_message( const std::string& message )
{
}

void PythonActionContext::report_need_resource( Core::NotifierHandle notifier )
{
	this->notifier_ = notifier;
	this->error_msg_ = std::string( "'" ) + notifier->get_name() + std::string(
	    "' is currently unavailable" );
}

void PythonActionContext::report_result( const Core::ActionResultHandle& result )
{
	this->result_ = result;
}

Core::ActionSource PythonActionContext::source() const
{
	switch ( this->action_mode_ )
	{
	case PythonActionMode::BATCH_E:
		return Core::ActionSource::SCRIPT_E;
	case PythonActionMode::REPLAY_E:
		return Core::ActionSource::PROVENANCE_E;
	default:
		return Core::ActionSource::COMMANDLINE_E;
	}
}

void PythonActionContext::set_action_mode( PythonActionMode mode )
{
	this->action_mode_ = mode;
}

Core::NotifierHandle PythonActionContext::get_resource_notifier()
{
	return this->notifier_;
}

Core::ActionResultHandle PythonActionContext::get_result()
{
	return this->result_;
}

void PythonActionContext::reset_context()
{
	this->notifier_.reset();
	this->result_.reset();
	this->error_msg_.clear();
}

std::string PythonActionContext::get_error_message()
{
	return this->error_msg_;
}


} //end namespace Seg3D
