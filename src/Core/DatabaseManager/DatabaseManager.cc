/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/Utils/StringUtil.h>
#include <Core/DatabaseManager/DatabaseManager.h>
#include <Core/Utils/Exception.h>
#include <Core/Utils/Lockable.h>
#include <sqlite3.h>

using namespace SCIRun::Core::Database;

namespace SCIRun
{
  namespace Core
  {
    namespace Database
    {

class DatabaseManagerPrivate : public SCIRun::Core::RecursiveLockable 
{
public:
	sqlite3* database_;
};
    }}}

DatabaseManager::DatabaseManager() :
	private_( new DatabaseManagerPrivate )
{
	// Open a default database
	int result = sqlite3_open( ":memory:", &this->private_->database_ );
	if ( result != SQLITE_OK )
	{
		this->private_->database_ = 0;
	}
	else
	{
		// Enable foreign key
		std::string error;
		this->run_sql_statement( "PRAGMA foreign_keys = ON;", error );
	}
}

DatabaseManager::~DatabaseManager()
{	
	// We need to close the database to avoid memory leak.
	if ( this->private_->database_ )
	{
		sqlite3_close( this->private_->database_ );
	}
}

static int InternalExecuteSqlStatement( sqlite3_stmt* statement, ResultSet& results )
{
	assert( statement != NULL );

	int result;
	while ( ( result = sqlite3_step( statement ) ) == SQLITE_ROW )
	{
		std::map< std::string, boost::any > temp_map;
		for( int j = 0; j < sqlite3_column_count( statement ); ++j )
		{
			boost::any temp_any;
			switch( sqlite3_column_type( statement, j ) )
			{
			case SQLITE_TEXT:
			case SQLITE_BLOB:
				{
					std::string string_result = std::string( reinterpret_cast< const char* >( 
						sqlite3_column_text( statement, j ) ) );
					temp_any = boost::any( string_result );
					break;
				}
			case SQLITE_INTEGER:
				{
					long long int_result = sqlite3_column_int64( statement, j );
					temp_any = boost::any( int_result );
					break;
				}
			case SQLITE_FLOAT:
				{
					double double_result = sqlite3_column_double( statement, j );
					temp_any = boost::any( double_result );
					break;
				}
			case SQLITE_NULL:
			default:
				break;
			}
			std::string column_name = std::string( sqlite3_column_name( statement, j ) );
			temp_map[ column_name ] = temp_any;
		}
		results.push_back( temp_map );
	}

	sqlite3_finalize( statement );

	return result;
}

bool DatabaseManager::run_sql_statement( const std::string& sql_str, std::string& error )
{
	ResultSet dummy_results;
	return this->run_sql_statement( sql_str, dummy_results, error );
}

bool DatabaseManager::run_sql_statement( const std::string& sql_str, ResultSet& results, 
	std::string& error )
{
	results.clear();

	DatabaseManagerPrivate::lock_type lock( this->private_->get_mutex() );

	if ( this->private_->database_ == NULL )
	{
		error = "Invalid database connection.";
		return false;
	}

	sqlite3_stmt* statement = NULL;
	if ( sqlite3_prepare_v2( this->private_->database_, sql_str.c_str(), 
		static_cast< int >( sql_str.size() ), &statement, NULL ) != SQLITE_OK )
	{
		error =  "The SQL statement '" + sql_str + "' failed to compile with error: "
			+ sqlite3_errmsg( this->private_->database_ );
		return false;
	}

	if( InternalExecuteSqlStatement( statement, results ) != SQLITE_DONE )
	{
		error =  "The SQL statement '" + sql_str + "' returned error: "
			+ sqlite3_errmsg( this->private_->database_ );
		return false;
	} 

	return true;
}

bool DatabaseManager::run_sql_script( const std::string& sql_str, std::string& error )
{
	DatabaseManagerPrivate::lock_type lock( this->private_->get_mutex() );

	if ( this->private_->database_ == NULL )
	{
		error = "Invalid database connection.";
		return false;
	}

	ResultSet dummy_results;
	const char* head = sql_str.c_str();
	const char* tail = NULL;
	// The input string length including the null terminator
	int num_bytes = static_cast< int >( sql_str.size() + 1 ); 
	while ( num_bytes > 1 )
	{
		sqlite3_stmt* statement = NULL;
		if ( sqlite3_prepare_v2( this->private_->database_, head, 
			num_bytes, &statement, &tail ) != SQLITE_OK )
		{
			error =  "The SQL statement '" + std::string( head ) + "' failed to compile with error: "
				+ sqlite3_errmsg( this->private_->database_ );
			return false;
		}

		if ( statement == NULL ) break;

		if( InternalExecuteSqlStatement( statement, dummy_results ) != SQLITE_DONE )
		{
			error =  "The SQL statement '" + std::string( head ) + "' returned error: "
				+ sqlite3_errmsg( this->private_->database_ );
			return false;
		} 

		num_bytes -= static_cast< int >( tail - head );
		head = tail;
	}

	return true;
}

bool DatabaseManager::load_database( const boost::filesystem::path& database_file, 
	std::string& error )
{
	DatabaseManagerPrivate::lock_type lock( this->private_->get_mutex() );

	int result;
	sqlite3* temp_open_database;
	sqlite3_backup* backup_database_object;
	
	result = sqlite3_open( database_file.string().c_str(), &temp_open_database );
	
	if ( result != SQLITE_OK ) 
	{
		sqlite3_close( temp_open_database );
		error = std::string( "Could not open database file '" ) + database_file.string() + "'.";
		return false;
	}
	
	backup_database_object = 
		sqlite3_backup_init( this->private_->database_, "main", temp_open_database, "main" );
	
	if ( backup_database_object )
	{
		sqlite3_backup_step( backup_database_object, -1 );
		sqlite3_backup_finish( backup_database_object );
	}
	
	sqlite3_close( temp_open_database );

	result = sqlite3_errcode( this->private_->database_ );
	if ( result != SQLITE_OK ) 
	{
		error = "Internal error in database.";
		return false;
	}
	
	// Enable foreign key
	this->run_sql_statement( "PRAGMA foreign_keys = ON;", error );

	error = "";
	return true;	
}


bool DatabaseManager::save_database( const boost::filesystem::path& database_file, 
	std::string& error )
{
	DatabaseManagerPrivate::lock_type lock( this->private_->get_mutex() );
	int result;
	sqlite3* temp_open_database;
	sqlite3_backup* backup_database_object;
	
	result = sqlite3_open( database_file.string().c_str(), &temp_open_database );
	
	if ( result != SQLITE_OK ) 
	{
		sqlite3_close( temp_open_database );
		error = std::string( "Could not open database file '" ) + database_file.string() + "'.";
		return false;
	}
	
	backup_database_object = 
		sqlite3_backup_init( temp_open_database, "main", this->private_->database_, "main" );
	
	if ( backup_database_object )
	{
		sqlite3_backup_step( backup_database_object, -1 );
		sqlite3_backup_finish( backup_database_object );
	}
	
	result = sqlite3_errcode( temp_open_database );
	if ( result != SQLITE_OK ) 
	{
		error = "Internal error in database.";
		return false;
	}
	
	sqlite3_close( temp_open_database );

	error = "";
	return true;	
}

long long DatabaseManager::get_last_insert_rowid()
{
	if ( this->private_->database_ != 0 )
	{
		return sqlite3_last_insert_rowid( this->private_->database_ );
	}

	return 0;
}

bool DatabaseManager::get_column_metadata( const std::string& table_name, 
										  const std::string& column_name, char const** data_type /*= NULL*/, 
										  char const** coll_seq /*= NULL*/, int* not_null /*= NULL*/, 
										  int* primary_key /*= NULL*/, int* auto_inc /*= NULL */ )
{
	if ( this->private_->database_ == 0 )
	{
		return false;
	}

	return sqlite3_table_column_metadata( this->private_->database_, "main", table_name.c_str(),
		column_name.c_str(), data_type, coll_seq, not_null, primary_key, auto_inc ) == SQLITE_OK;
}

std::string DatabaseManager::EscapeQuotes( const std::string& str )
{
	std::string result;
	std::string tmp_str = str;
	std::string::size_type pos = str.find( '\'' );
	while ( pos != std::string::npos )
	{
		result += tmp_str.substr( 0, pos );
		result += "''";
		tmp_str = tmp_str.substr( pos + 1 );
		pos = tmp_str.find( '\'' );
	}
	result += tmp_str;

	return result;
}
