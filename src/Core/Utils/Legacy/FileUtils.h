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


///
///@file FileUtils.h
///
///@author
///      Chris Moulding
///      University of Utah
///@date Sept 2000
///

#ifndef FILEUTILS_H
#define FILEUTILS_H 1

#include <map>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <Core/Utils/Legacy/share.h>

namespace SCIRun {

////////////////////////////////////
/// InsertStringInFile()
/// Inserts "insert" in front of all occurrances of
/// "match" within the file named "filename"

SCISHARE void InsertStringInFile(char* filename, const char* match, const char* insert);


////////////////////////////////////
/// GetFilenamesEndingWith()
/// returns a std::map of strings that contains
/// all the files with extension "ext" inside
/// the directory named "dir"

SCISHARE std::map<int,char*>* GetFilenamesEndingWith(const char* dir,
                                                     const char* ext);

SCISHARE std::vector<std::string> GetFilenamesStartingWith(const std::string & dir,
                                                      const std::string & prefix);

/// Return all of the filenames that appear to be in the same numeric
/// sequence as the reference one.  This means all the letters in each
/// are the same and only the numbers are different.  No sorting is done.
SCISHARE std::vector<std::string> GetFilenamesInSequence(const std::string &dir,
                                                    const std::string &ref);

SCISHARE std::pair<std::string, std::string> split_filename(std::string fname);

SCISHARE std::string findFileInPath(const std::string &filename,
                                    const std::string &path);

SCISHARE bool validFile( const std::string& filename );
SCISHARE bool validFile( const boost::filesystem::path& filename );

SCISHARE bool validDir( const std::string& dirname );
SCISHARE bool validDir( const boost::filesystem::path& dirname );

SCISHARE bool isSymLink( const boost::filesystem::path& filename );

// Creates a temp file (in directoryPath), writes to it, checks the resulting files size, and then deletes it...
SCISHARE bool testFilesystem( std::string directoryPath );

SCISHARE std::string autocomplete(const std::string &);
SCISHARE std::string canonicalize(std::string);
SCISHARE std::string substituteTilde(const std::string &dirstr);

// Replaces '/' with '\' or vice-versa between unix and windows paths
SCISHARE void convertToWindowsPath( std::string & unixPath );
inline void convertToUnixPath( std::string & winPath )
{
  std::replace(winPath.begin(), winPath.end(), '\\', '/');
}

SCISHARE int copyFile(const boost::filesystem::path& src,
                      const boost::filesystem::path& dest);
SCISHARE int moveFile(const boost::filesystem::path& src,
                      const boost::filesystem::path& dest);
SCISHARE int deleteFile(const boost::filesystem::path& filename);
/// @todo: re-implement with boost
// The strings are not references since windows has to convert
// '/' to '\\', and we do that in the same string
SCISHARE int copyDir(std::string src, std::string dest);
SCISHARE int deleteDir(const boost::filesystem::path& filename);

// Count the number of non-empty, non-whitespace-only lines in file
SCISHARE int getNumNonEmptyLines(char *filename);

} // End namespace SCIRun

#ifdef _WIN32
// windows doesn't have dirent... make them here
struct dirent
{
    char *d_name;
};

struct DIR;

SCISHARE DIR *opendir(const char *);
SCISHARE int closedir(DIR *);
SCISHARE dirent *readdir(DIR *);

// not implemented yet...
SCISHARE void rewinddir(DIR *);
#endif

#endif
