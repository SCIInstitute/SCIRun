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

#include <Python.h>

#include <string>
#include <vector>

#include <boost/python.hpp>

#include <Core/Geometry/Color.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Utils/Variant.h>

#include <Core/Python/ToPythonConverters.h>

template< class T >
class StdVectorToListConverter : public boost::python::converter::wrap_pytype< &PyList_Type >
{
public:
	static PyObject* convert( const std::vector< T >& v )
	{
		boost::python::list result;
		for ( size_t i = 0; i < v.size(); ++i )
		{
			result.append( v[ i ] );
		}

		return boost::python::incref( result.ptr() );
	}
};

template< class T >
class TripleToListConverter : public boost::python::converter::wrap_pytype< &PyList_Type >
{
public:
	static PyObject* convert( const T& v )
	{
		boost::python::list result;
		for ( size_t i = 0; i < 3; ++i )
		{
			result.append( v[ i ] );
		}

		return boost::python::incref( result.ptr() );
	}
};

template< class T >
T GetValueFromVarient( const Core::Variant& v )
{
	T result;
	bool succeed = const_cast< Core::Variant& >( v ).get< T >( result );
	assert( succeed );
	return result;
}

class VariantToPythonConverter
{
public:
	static PyObject* convert( const Core::Variant& v )
	{
		boost::python::object result;
		const std::type_info& value_type = v.internal_type();
		if ( value_type == typeid( bool ) )
		{
			result = boost::python::object( GetValueFromVarient< bool >( v ) );
		}
		else if ( value_type == typeid( int ) )
		{
			result = boost::python::object( GetValueFromVarient< int >( v ) );
		}
		else if ( value_type == typeid( long long ) )
		{
			result = boost::python::object( GetValueFromVarient< long long >( v ) );
		}
		else if ( value_type == typeid( double ) )
		{
			result = boost::python::object( GetValueFromVarient< double >( v ) );
		}
		else if ( value_type == typeid( Core::Color ) )
		{
			result = boost::python::object( GetValueFromVarient< Core::Color >( v ) );
		}
		else if ( value_type == typeid( Core::Point ) )
		{
			result = boost::python::object( GetValueFromVarient< Core::Point >( v ) );
		}
		//else if ( value_type == typeid( std::vector< bool > ) )
		//{
		//	result = boost::python::object( GetValueFromVarient< std::vector< bool > >( v ) );
		//}
		else if ( value_type == typeid( std::vector< double > ) )
		{
			result = boost::python::object( GetValueFromVarient< std::vector< double > >( v ) );
		}
		else if ( value_type == typeid( std::vector< std::string > ) )
		{
			result = boost::python::object( GetValueFromVarient< std::vector< std::string > >( v ) );
		}
		//else if ( value_type == typeid( std::vector< Core::Color > ) )
		//{
		//	result = boost::python::object( GetValueFromVarient< std::vector< Core::Color > >( v ) );
		//}
		else if ( value_type == typeid( std::vector< Core::Point > ) )
		{
			result = boost::python::object( GetValueFromVarient< std::vector< Core::Point > >( v ) );
		}
		else
		{
			result = boost::python::object( v.export_to_string() );
		}

		return boost::python::incref( result.ptr() );
	}
};

namespace Core
{

void RegisterToPythonConverters()
{
	boost::python::to_python_converter< std::vector< std::string >, 
		StdVectorToListConverter< std::string >, true >();
	boost::python::to_python_converter< std::vector< bool >,
		StdVectorToListConverter< bool >, true >();
	boost::python::to_python_converter< std::vector< double >,
		StdVectorToListConverter< double >, true >();

	boost::python::to_python_converter< Core::Color,
		TripleToListConverter< Core::Color >, true >();
	boost::python::to_python_converter< Core::Point,
		TripleToListConverter< Core::Point >, true >();
	boost::python::to_python_converter< Core::PointF,
		TripleToListConverter< Core::PointF >, true >();
	boost::python::to_python_converter< Core::Vector,
		TripleToListConverter< Core::Vector >, true >();
	boost::python::to_python_converter< Core::VectorF,
		TripleToListConverter< Core::VectorF >, true >();

	boost::python::to_python_converter< Core::Variant, VariantToPythonConverter >();
}

} // end namespace Core