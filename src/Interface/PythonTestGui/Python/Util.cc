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

#ifdef _MSC_VER
#pragma warning( disable: 4244 4267 )
#endif

#include <boost/python.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/slice.hpp>

#include <string>

#include <Core/Action/ActionFactory.h>
#include <Core/Action/ActionDispatcher.h>
#include <Core/Python/Util.h>
#include <Core/Python/PythonInterpreter.h>

namespace Core
{

boost::python::object RunActionFromPython( boost::python::tuple args,
												  boost::python::dict kw_args )
{
	std::string action_str;
	boost::python::ssize_t num_of_args = boost::python::len( args );
	for ( boost::python::ssize_t i = 0; i < num_of_args; ++i )
	{
		action_str += boost::python::extract< std::string >( 
			boost::python::str( args[ i ] ) + " " )();
	}

	boost::python::list kw_pairs = kw_args.items();
	num_of_args = boost::python::len( kw_pairs );
	for ( boost::python::ssize_t i = 0; i < num_of_args; ++i )
	{
		action_str += boost::python::extract< std::string >( 
			boost::python::str( kw_pairs[ i ][ 0 ] ) + "=" + 
			boost::python::str( kw_pairs[ i ][ 1 ] ) + " " )();
	}

	std::string action_error;
	std::string action_usage;

	Core::ActionHandle action;

	if ( !Core::ActionFactory::CreateAction( action_str, action, action_error, action_usage ) )
	{
		action_error = action_error + "\nUsage: " + action_usage;
		PyErr_SetString( PyExc_Exception, action_error.c_str() );
	}
	else
	{
		PythonActionContextHandle action_context = PythonInterpreter::GetActionContext();

		bool repeat = true;
		do 
		{
			Core::ActionDispatcher::PostAndWaitAction( action, Core::ActionContextHandle( action_context ) );
			Core::ActionStatus action_status = action_context->status();
			Core::ActionResultHandle action_result = action_context->get_result();
			Core::NotifierHandle resource_notifier = action_context->get_resource_notifier();
			Core::ActionSource action_source = action_context->source();
			std::string err_msg = action_context->get_error_message();
			action_context->reset_context();

			if ( action_status == Core::ActionStatus::SUCCESS_E )
			{
				// Wait for the resource if currently running in script mode
				if ( resource_notifier && ( action_source == Core::ActionSource::SCRIPT_E ||
					action_source == Core::ActionSource::PROVENANCE_E ) )
				{
					resource_notifier->wait();
				}

				if ( action_result )
				{
					return boost::python::object( *action_result );
				}
				else
				{
					return boost::python::object( true );
				}
				repeat = false;
			}
			else
			{
				if ( resource_notifier && ( action_source == Core::ActionSource::SCRIPT_E ||
					action_source == Core::ActionSource::PROVENANCE_E ) )
				{
					resource_notifier->wait();
					continue;
				}
				PyErr_SetString( PyExc_Exception, err_msg.c_str() );
				repeat = false;
			}
		} while ( repeat );
	}

	return boost::python::object();
}

} // end namespace Core