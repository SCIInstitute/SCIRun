
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

#ifndef CORE_UTIL_DIR_H
#define CORE_UTIL_DIR_H


#include <string>
#include <vector>


#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#define S_IRUSR 0x0100
#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define	S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#endif

#include <Core/Utils/Legacy/share.h>
namespace SCIRun {
   
   /**************************************
     
     @class
       Dir
      
       Short Description...
      
     GENERAL INFORMATION
      
       Dir.h
     @author 
       Steven G. Parker
       Department of Computer Science
       University of Utah
      
     KEYWORDS
       Dir
      
     DESCRIPTION
       Long description...
      
     WARNING
      
     ****************************************/
    
   class SCISHARE Dir {
   public:
      Dir();
      Dir(const Dir&);
      Dir(const std::string&);
      ~Dir();
      Dir& operator=(const Dir&);

      static Dir create(const std::string& name);
			static Dir current_directory();
      
      void remove(bool throwOnError = true);
    
      // removes even if the directory has contents
      void forceRemove(bool throwOnError = true);

      // remove a file
      void remove(const std::string& filename, bool throwOnError = true);

      // copy this directory to under the destination directory
      void copy(Dir& destDir);
      void move(Dir& destDir);

      // copy a file in this directory to the destination directory
      void copy(const std::string& filename, Dir& destDir);
      void move(const std::string& filename, Dir& destDir);

      Dir createSubdir(const std::string& name);
      Dir getSubdir(const std::string& name);
      bool exists();
      bool isfile();

     void getFilenamesBySuffix(const std::string& suffix,
			       std::vector<std::string>& filenames);

      std::string getName() const {
	 return name_;
      }
      // Delete the dir and all of its files/sub directories using C++ calls.
      //
      static bool removeDir( const char * dirName );


   private:

      std::string name_;
   };
} // End namespace SCIRun

#ifdef _WIN32
// windows mkdir doesn't take permissions
#  define MKDIR(dir, perm) mkdir(dir)
// windows doesn't have lstat
#  define LSTAT(file, buf) stat(file, buf)

#else
#  define MKDIR(dir, perm) mkdir(dir, perm)
#  define LSTAT(file, buf) lstat(file, buf)
#endif

#endif
