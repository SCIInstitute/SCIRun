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


/// @todo: NEED TO GO OVER THESE FUNCTIONS
// THIS CODE CONTAINS SOME REALLY SCARY CONSTRUCTS AND SHOULD BE REPLACED
// BY BETTER CODE

/*@file FileUtils.cc */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef _WIN32
#  include <unistd.h>
#  include <dirent.h>
#else
#  include <io.h>
   typedef unsigned short mode_t;
#  define MAXPATHLEN 256
#endif
#include <sys/stat.h>
#include <Core/Utils/Legacy/FileUtils.h>
#include <Core/Utils/Legacy/Assert.h>
#include <Core/Utils/Legacy/Environment.h>
#include <Core/Utils/Legacy/sci_system.h>
#include <Core/Utils/Legacy/StringUtil.h>

#include <boost/system/error_code.hpp>

#include <sci_debug.h>

#include <iostream>

namespace bfs=boost::filesystem;
typedef bfs::path::string_type boost_string_type;
namespace bsys=boost::system;

namespace SCIRun {

////////////////////////////////////////////////////////////
//
// InsertStringInFile
//
//       If "match" is found in "filename", then add "add_text" text
//       to the file _after_ the location of the matched text.
//
//   Normally, I would just use sed via system() to edit a file,
//   but for some reason system() calls never work from Dataflow
//   processes in linux.  Oh well, sed isn't natively available
//   under windows, so I'd have to do something like this anyhow
//   - Chris Moulding

void
InsertStringInFile(char* filename, const char* match, const char* add_text)
{
  /// @todo: rewrite in C++
  char* newfilename = new char[strlen(filename)+2];
  char c;
  sprintf(newfilename,"%s~",filename);
  FILE* ifile;
  FILE* ofile;

  /* create a copy of the original file */
  ifile = fopen(filename,"r");

  if( ifile == NULL ) {
    printf( "ERROR: In Core/Util/FileUtils.cc: InsertStringInFile:\n" );
    printf("        File '%s' does not exist!\n", filename );
    printf( "       There is something seriously wrong with your SCIRun installation.\n");
    printf( "       Please contact scirun@sci.utah.edu.\n");
    exit( 1 );
  }

  ofile = fopen(newfilename,"w");

  c = (char)fgetc(ifile);
  while (c!=(char)EOF) {
    fprintf(ofile,"%c",c);
    c = (char)fgetc(ifile);
  }
  fclose(ifile);
  fclose(ofile);

  // Search the copy for an instance of "match"...
  int index1 = 0;
  unsigned int index2 = 0;
  int foundat = -1;
  ifile = fopen(newfilename,"r");
  c = (char)fgetc(ifile);
  while (c!=(char)EOF) {
    if (c==match[index2]) {
      foundat = index1 + strlen(match);
      while (index2<strlen(match) && c!=(char)EOF && c==match[index2]) {
        c = (char)fgetc(ifile);
        index1++;
        index2++;
      }
      if( foundat >= 0 && index2 != strlen(match) ) {
        foundat = -1;
        index2 = 0;
      } else {
        break;
      }
    }
    c = (char)fgetc(ifile);
    index1++;
  }
  fclose(ifile);

  // If an instance of match was found, insert the indicated string...
  if( foundat >= 0 ) {
    index1 = 0;
    ifile = fopen(newfilename,"r");
    ofile = fopen(filename,"w");
    c = (char)fgetc(ifile);
    while (c!=(char)EOF) {
      if( index1 == foundat ) {
        fprintf( ofile, "%s", add_text );
      }
      fprintf(ofile,"%c",c);
      c = (char)fgetc(ifile);
      index1++;
    }
    fclose(ifile);
    fclose(ofile);
  }
}

std::map<int,char*>*
GetFilenamesEndingWith(const char* d, const  char* ext)
{
  std::map<int,char*>* newmap = 0;
  dirent* file = 0;
  DIR* dir = opendir(d);
  char* newstring = 0;

  if (!dir)
    return 0;

  newmap = new std::map<int,char*>;

  file = readdir(dir);
  while (file) {
    if ((strlen(file->d_name)>=strlen(ext)) &&
        (strcmp(&(file->d_name[strlen(file->d_name)-strlen(ext)]),ext)==0)) {
      newstring = new char[strlen(file->d_name)+1];
      sprintf(newstring,"%s",file->d_name);
      newmap->insert(std::pair<int,char*>(newmap->size(),newstring));
    }
    file = readdir(dir);
  }

  closedir(dir);
  return newmap;
}

std::string
substituteTilde( const std::string & dirstr ) {
  std::string realdirstr = dirstr;

  std::string::size_type pos = realdirstr.find("~");
  if (pos != std::string::npos && (pos == 0 || dirstr[pos-1] == '/')) {
    std::string HOME = sci_getenv("HOME");
    realdirstr = HOME + "/" +
      dirstr.substr(pos+1, dirstr.size()-pos-1);
  }
  return realdirstr;
}

std::vector<std::string>
GetFilenamesStartingWith(const std::string &dirstr,
                         const std::string &prefix)
{
  std::vector<std::string> files(0);
  DIR* dir = opendir(substituteTilde(dirstr).c_str());
  if (!dir) {
    return files;
  }

  dirent* file = readdir(dir);
  while (file) {
    ASSERT(file->d_name);
    std::string dname = file->d_name;
    std::string::size_type pos = dname.find(prefix);
    if (pos == 0) {
      files.push_back(dname);
    }
    file = readdir(dir);
  }

  closedir(dir);
  return files;
}


static bool
in_sequence_compare(const std::string &a, const std::string &b)
{
  if (a.size() != b.size()) return false;

  for (size_t i = 0; i < a.size(); i++)
  {
    if (!(
        a[i] == b[i] ||
        (isdigit(a[i]) && isdigit(b[i]))
      ))
      return false;
  }
  return true;
}


std::vector<std::string>
GetFilenamesInSequence(const std::string &dirstr, const std::string &reffile)
{
  std::vector<std::string> files;
  DIR* dir = opendir(substituteTilde(dirstr).c_str());
  if (!dir) {
    return files;
  }

  dirent* file = readdir(dir);
  while (file) {
    ASSERT(file->d_name);
    std::string dname = file->d_name;
    if (in_sequence_compare(dname, reffile))
    {
      files.push_back(dname);
    }
    file = readdir(dir);
  }

  closedir(dir);
  return files;
}


std::pair<std::string, std::string>
split_filename(std::string fname) {
  if (fname[fname.size()-1] == '/' || fname[fname.size()-1] == '\\') {
    fname = fname.substr(0, fname.size()-1);
  }

  if (validDir(fname)) {
    return make_pair(fname, std::string(""));
  }

  std::string::size_type pos = fname.find_last_of("/");
  if (pos == std::string::npos)
    pos = fname.find_last_of("\\");
  std::pair<std::string, std::string> dirfile = std::make_pair
    (fname.substr(0, pos+1), fname.substr(pos+1, fname.length()-pos-1));

  return dirfile;
}

bool
validFile( const bfs::path& filename )
{
  bool retval = false;
  try
  {
    retval = bfs::is_regular_file(filename);
  }
  catch (const bfs::filesystem_error& ex)
  {
    std::cerr << "boost::filesystem::is_regular_file failed in " << __FILE__ << " " << __LINE__ << ": " << ex.what() << std::endl;
  }
  return retval;
}

bool
validFile( const std::string& filename )
{
  bool retval = false;
  try {
    std::string updatedFilename = substituteTilde(filename);
    retval = bfs::is_regular_file(filename);
  }
  catch (const bfs::filesystem_error& ex)
  {
    std::cerr << "boost::filesystem::is_regular_file failed in " << __FILE__ << " " << __LINE__ << ": " << ex.what() << std::endl;
  }
  return retval;
}

bool
validDir( const bfs::path& dirname )
{
  bool retval = false;
  try
  {
    retval = bfs::is_directory(dirname);
  }
  catch (const bfs::filesystem_error& ex)
  {
    std::cerr << "boost::filesystem::is_directory failed in " << __FILE__ << " " << __LINE__ << ": " << ex.what() << std::endl;
  }
  return retval;
}

bool
validDir( const std::string& dirname )
{
  bool retval = false;
  try {
    std::string updatedDirname = substituteTilde(dirname);
    retval = bfs::is_directory(updatedDirname);
  }
  catch (const bfs::filesystem_error& ex)
  {
    std::cerr << "boost::filesystem::is_directory failed in " << __FILE__ << " " << __LINE__ << ": " << ex.what() << std::endl;
  }
  return retval;
}

bool
isSymLink( const bfs::path& filename )
{
  bool retval = false;
  try
  {
    retval = bfs::is_symlink(filename);
  }
  catch (const bfs::filesystem_error& ex)
  {
    std::cerr << "boost::filesystem::is_symlink failed in " << __FILE__ << " " << __LINE__ << ": " << ex.what() << std::endl;
  }
  return retval;
}

// Creates a temp file (in directoryPath), writes to it, and then deletes it...
bool
testFilesystem( std::string directoryPath )
{
  FILE * fp;

  std::string fileName = directoryPath + "/scirun_filesystem_check_temp_file";

  // Create a temporary file
  fp = fopen( fileName.c_str(), "w" );
  if( fp == NULL ) {
    printf( "ERROR: testFilesystem() failed to create a temporary file in %s\n", directoryPath.c_str() );
    printf( "       errno is %d\n", errno );
      return false;
  }

  // Write to the file
  const char * myStr = "hello world";
  for( int cnt = 0; cnt < 1000; cnt++ ) {
    int numWritten = fwrite( myStr, 1, 11, fp );
    if( numWritten != 11 ) {
      printf( "ERROR: testFilesystem() failed to write data to temp file in %s\n", directoryPath.c_str() );
      printf( "       iteration: %d, errno is %d\n", cnt, errno );
      return false;
    }
  }

  // Close the file
  int result = fclose( fp );
  if (result != 0) {
    printf( "WARNING: fclose() failed while testing filesystem.\n" );
    printf( "         errno is %d\n", errno );
    return false;
  }

  // Check the files size
  struct stat buf;
  if( stat(fileName.c_str(), &buf) == 0 )
  {
    // Using stl to avoid having to mess with the type strings for printf.
//    std::cerr << "FILESYSTEM CHECK: Test file size is: "<<buf.st_size<<"\n";
  } else {
    printf( "WARNING: stat() failed while testing filesystem.\n" );
    printf( "         errno is %d\n", errno );
    return false;
  }

  // Delete the file
  int rc = remove( fileName.c_str() );
  if (rc != 0) {
    printf( "WARNING: remove() failed while testing filesystem.\n" );
    printf( "         errno is %d\n", errno );
    return false;
  }
  return true;
}


// findFileInPath
// Searches the colon-seperated 'path' string variable for a file named
// 'file'.  From left to right in the path string each directory is
// tested to see if the file named 'file' is in it.
//
// If the file is found, it returns the DIRECTORY that the file is located in
// Otherwise if the file is not found in the path, returns an empty string
//
// If the file is found in multiple directories in the 'path', only
// the first matching directory is returned
std::string
findFileInPath(const std::string &file, const std::string &path)
{
  std::string::size_type beg = 0;
  std::string::size_type end = 0;

  while (beg < path.length()) {
#ifdef _WIN32
    end = path.find(":",beg+2);
#else
    end = path.find(":",beg);
#endif
    if (end == std::string::npos) end = path.size();

    std::string dir(path, beg, end-beg);
    ASSERT(!dir.empty());
    // Append a slash if there isn't one
    if (validDir(dir)) {
      if (dir[dir.length()-1] != '/') dir = dir + "/";
      std::string filename = dir + file;
      if (validFile(filename)) {
        // see comments at function start
        return dir;
      }
    }

    beg = end+1;
  }
  return "";
}

std::string
autocomplete(const std::string &instr) {
  std::string str = instr;
  std::pair<std::string, std::string> dirfile = split_filename(str);
  std::string dir = dirfile.first;
  if (!validDir(dir)) {
#ifdef _WIN32
    // in windows c:/dir1/ will not be valid, but c:/dir1 will be.  Lose the end '/' and try again
    if ((dir[dir.length()-1] == '/' || dir[dir.length()-1] == '\\') && dir.length() > 1) {
      dir = dir.substr(0, dir.length()-1);
      if (!validDir(dir))
        return str;
    }
    else
#endif
      return str;
  }
  if (dirfile.first.empty() || dirfile.first[dirfile.first.length()-1] != '/') {
    dirfile.first = dirfile.first+"/";
  }
    std::vector<std::string> files =
    GetFilenamesStartingWith(dir, dirfile.second);


  if (files.empty()) {
    return str;
  } if (files.size() == 3 && files[0] == "." && files[1] == "..") {
    str = dirfile.first + files[2];
    if (validDir(str)) {
      str = str + "/";
    }
  } else if (files.size() == 1) {
    str = dirfile.first + files[0];
    if (validDir(str)) {
      str = str + "/";
    }
  } else {
    unsigned int j0 = dirfile.second.size();
    unsigned int j = j0;
    do {
      for (unsigned int i = 1; i < files.size(); ++i) {
        if ((j == files[i].size()) || (files[i][j] != files[i-1][j])) {
          str = str + files[i].substr(j0, j-j0);
          return str;
        }
      }
      ++j;
    } while (1);
  }
  return str;
}


std::string
canonicalize(std::string filename)
{
  filename = substituteTilde(filename);

  // use unix paths internally to keep things simpler
  convertToUnixPath(filename);
  filename = replace_substring(filename, "//", "/");
  std::vector<std::string> entries, newentries = split_string(filename, '/');
  while (newentries.size() != entries.size()) {
    entries = newentries;
    newentries.clear();
    for (unsigned int i = 0; i < entries.size(); ++i) {
      if ( (entries[i] != ".") && // Skip . entries
           (entries[i] != "..") && // Skil .. entries
           (i+1 == entries.size() || entries[i+1] != "..")) // Skip entries parenting .. entries
      {
        newentries.push_back(entries[i]);
      }
    }
  }

  filename = "";
  for (unsigned int i = 0; i < entries.size(); ++i) {
#ifdef _WIN32
    // don't put / in front of c:\ as it doesn't make sense
    if (i == 0 && entries[i].size() > 1 && entries[i][1] == ':')
      filename = entries[i];
    else
#endif
    filename = filename + "/" + entries[i];
  }
  if (filename == "" || validDir(filename)) {
    filename = filename+"/";
  }

  return filename;
}



void
convertToWindowsPath( std::string & unixPath )
{
  for( std::string::size_type cnt = 0; cnt < unixPath.length(); cnt++ ) {
    if( unixPath[cnt] == '/' ) {
      unixPath[cnt] = '\\';
    }
  }
}

int copyFile(const bfs::path& src, const bfs::path& dest)
{
  int code = -1;
  try
  {
    bsys::error_code ec;
    bfs::copy_file(src, dest, bfs::copy_option::overwrite_if_exists, ec);
    code = ec.value();
#if DEBUG
    if (code != bsys::errc::success)
    {
      std::cerr << "error " << ec.value() << ": " << ec.message() << std::endl;
    }
#endif
  }
  catch (...)
  {
    std::cerr << "Error copying " << src.c_str() << " to " << dest.c_str() << std::endl;
  }
  return code;
}


int moveFile(const bfs::path& src, const bfs::path& dest)
{
  int code = -1;
  try
  {
    bsys::error_code ec;
    bfs::rename(src, dest, ec);
    code = ec.value();
#if DEBUG
    if (code != bsys::errc::success)
    {
      std::cerr << "error " << ec.value() << ": " << ec.message() << std::endl;
    }
#endif
  }
  catch (...)
  {
    std::cerr << "Error moving " << src.c_str() << " to " << dest.c_str() << std::endl;
  }
  return code;
}

int deleteFile(const bfs::path& filename)
{
  int code = -1;
  try
  {
    bsys::error_code ec;
    if (! bfs::remove(filename, ec))
    {
      std::cerr << filename.c_str() << " was not found" << std::endl;
    }
    code = ec.value();
#if DEBUG
    if (code != bsys::errc::success)
    {
      std::cerr << "error " << ec.value() << ": " << ec.message() << std::endl;
    }
#endif
  }
  catch (...)
  {
    std::cerr << "Error removing " << filename.c_str() << std::endl;
  }
  return code;
}


/// @todo: boostify this
int copyDir(std::string src, std::string dest)
{
#ifdef _WIN32
  std::string cpCmd = "xcopy /Y ";
  convertToWindowsPath(src);
  convertToWindowsPath(dest);
#else
  std::string cpCmd = "cp -fr ";
#endif

  std::string cmd = cpCmd + "\"" + src + "\"" + " " + "\"" + dest + "\"";
  int code = sci_system(cmd.c_str());
  if (code)
  {
    std::cerr << "Error executing: " << cmd << "\n";
  }
  return code;
}

int deleteDir(const bfs::path& filename)
{
  int code = -1;
  try
  {
    bsys::error_code ec;
    bfs::remove_all(filename, ec);
    code = ec.value();
  }
  catch (...)
  {
    std::cerr << "Error removing " << filename.c_str() << std::endl;
  }
  return code;
}


int
getNumNonEmptyLines(char *filename)
{
  // read through the file -- when you see a non-white-space set a flag to one.
  // when you get to the end of the line (or EOF), see if the flag has
  // been set.  if it has, increment the count and reset the flag to zero.

  FILE *fin = fopen(filename, "rt");
  if (!fin) {
    std::cerr << "Error -- could not open file " << filename << "\n";
    exit(2);
  }
  int count = 0;
  int haveNonWhiteSpace = 0;
  int c;
  while ((c = fgetc(fin)) != EOF)
  {
    if (!isspace(c)) haveNonWhiteSpace = 1;
    else if ((c == '\n' || c == '\r') && haveNonWhiteSpace)
    {
      count++;
      haveNonWhiteSpace = 0;
    }
  }
  if (haveNonWhiteSpace) count++;

  return count;
}



} // End namespace SCIRun


#ifdef _WIN32
#  include <errno.h>
struct DIR
{
  intptr_t file_handle;
  _finddata_t finddata;
  _finddata_t nextfinddata;
  bool done;
  dirent return_on_read;
};

DIR *
opendir(const char *name)
{
  // grab the first file in the directory by ending name with "/*"
  DIR *dir = 0;
  if (name != 0)
  {
    size_t length = strlen(name);
    if (length > 0)
    {
      dir = new DIR;
      dir->done = false;

      // create the file search path with the wildcard
      std::string search_path = name;
      if (name[length-1] == '/' || name[length-1] == '\\')
        search_path += "*";
      else
        search_path += "/*";

      if ((dir->file_handle = _findfirst(search_path.c_str(), &dir->nextfinddata)) == -1)
      {
        delete dir;
        dir = 0;
      }
      return dir;
    }
  }
  errno = EINVAL;
  return 0;
}

int
closedir(DIR *dir)
{
  int result = -1;
  if (dir)
  {
    result = _findclose(dir->file_handle);
    delete dir;
  }
  if (result == -1) errno = EBADF;
  return result;
}

dirent
*readdir(DIR *dir)
{
  if (dir->done)
    return 0;
  if (dir)
  {
    dir->finddata = dir->nextfinddata;
    if (_findnext(dir->file_handle, &dir->nextfinddata) == -1)
      dir->done = true;
    dir->return_on_read.d_name = dir->finddata.name;
    return &dir->return_on_read;
  }
  else
  {
    errno = EBADF;
  }
  return 0;
}

void
rewinddir(DIR *dir)
{
  printf( "Warning: FileUtils.cc: rewinddir not implemented!\n" );
}


#endif
