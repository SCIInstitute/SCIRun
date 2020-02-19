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


///    @file    TypeDescription.cc
///    @author  Martin Cole
///    @date    Mon May 14 10:20:21 2001

#include <Core/Utils/Legacy/TypeDescription.h>
#include <Core/Utils/Legacy/FileUtils.h>

#include <Core/Exceptions/InternalError.h>
#include <Core/Utils/Legacy/Assert.h>
#include <Core/Thread/Mutex.h>
#include <Core/Utils/Legacy/MemoryUtil.h>
#include <map>
#include <iostream>

using namespace SCIRun::Core::Thread;

namespace SCIRun {

struct KillMap {
  ~KillMap();
};

static std::map<std::string, const TypeDescription*>* types = nullptr;
static std::vector<const TypeDescription*>* typelist=nullptr;
static Mutex typelist_lock("TypeDescription::typelist lock");
static bool killed=false;

KillMap::~KillMap()
{
  if(!types){
    ASSERT(!killed);
    ASSERT(!typelist);
    return;
  }
  killed=true;
  delete_all_items(*typelist);
  delete types;
  delete typelist;
}

KillMap killit;


void
TypeDescription::register_type()
{
  typelist_lock.lock();
  if (!types)
  {
    ASSERT(!killed);
    ASSERT(!typelist);

    // This will make sure that if types was not initialized when we
    // entered this block, that we will not try and reinitialize types
    // and typelist.
    if (!types)
    {
      types = new std::map<std::string, const TypeDescription*>;
      typelist = new std::vector<const TypeDescription*>;
    }
  }

  std::map<std::string, const TypeDescription*>::iterator iter = types->find(get_name());
  if (iter == types->end())
  {
    (*types)[get_name()] = this;
  }
  typelist->push_back(this);

  typelist_lock.unlock();
}


TypeDescription::TypeDescription(const std::string &name,
				 const std::string &path,
				 const std::string &namesp,
				 category_e c) :
  subtype_(nullptr),
  name_(name),
  h_file_path_(strip_absolute_path(path)),
  namespace_(namesp),
  category_(c)
{
  register_type();
}

TypeDescription::TypeDescription(const std::string &name,
				 td_vec* sub,
				 const std::string &path,
				 const std::string &namesp,
				 category_e c) :
  subtype_(sub),
  name_(name),
  h_file_path_(strip_absolute_path(path)),
  namespace_(namesp),
  category_(c)
{
  register_type();
}

TypeDescription::~TypeDescription()
{
  delete subtype_;
}

std::string
TypeDescription::get_name( const std::string & type_sep_start /* = "<"  */,
			   const std::string & type_sep_end   /* = "> " */ ) const
{
  const std::string comma(",");
  bool do_end = true;
  if(subtype_) {
    std::string rval = name_ + type_sep_start;
    td_vec::iterator iter = subtype_->begin();
    while (iter != subtype_->end()) {
      rval+=(*iter)->get_name(type_sep_start, type_sep_end);
      ++iter;
      if (iter != subtype_->end()) {
	if (type_sep_start == type_sep_end) {
	  do_end = false;
	} else {
	  rval += comma;
	}
      }
    }
    if (do_end) rval += type_sep_end;
    return rval;
  } else {
    return name_;
  }
}

// substitute one of the subtype names with the name provided.
std::string
TypeDescription::get_similar_name(const std::string &substitute,
				  const int pos,
				  const std::string &type_sep_start,
				  const std::string &type_sep_end) const
{
  const std::string comma(",");
  bool do_end = true;
  if(subtype_) {
    std::string rval = name_ + type_sep_start;
    td_vec::iterator iter = subtype_->begin();
    int count = 0;
    while (iter != subtype_->end()) {
      if (pos == count) {
	rval += substitute;
      } else {
	rval+=(*iter)->get_name(type_sep_start, type_sep_end);
      }
      ++iter;
      if (iter != subtype_->end()) {
	if (type_sep_start == type_sep_end) {
	  do_end = false;
	} else {
	  rval += comma;
	}
      }
      ++count;
    }
    if (do_end) rval += type_sep_end;
    return rval;
  } else {
    return name_;
  }
}


std::string
TypeDescription::get_filename() const
{
  std::string s = get_name();
  std::string result;
  for (unsigned int i = 0; i < s.size(); i++)
  {
    if (isalnum(s[i]))
    {
      result += s[i];
    }
  }
  return result;
}

const TypeDescription*
TypeDescription::lookup_type(const std::string& t)
{
  if(!types) {
    typelist_lock.lock();
    if (!types) {
      types=new std::map<std::string, const TypeDescription*>;
      typelist=new std::vector<const TypeDescription*>;
    }
    typelist_lock.unlock();
  }

  auto iter = types->find(t);
  if(iter == types->end())
     return nullptr;
  return iter->second;
}

std::string TypeDescription::cc_to_h(const std::string &dot_cc)
{
  const size_t len = dot_cc.length();
  std::string dot_h;
  if (len > 3 && dot_cc.substr(len-3, len) == ".cc") {
    dot_h = dot_cc.substr(0, len-3) + ".h";
  } else {
    std::cerr << "Warning: TypeDescription::cc_to_h input does not end in .cc"
	 << std::endl << "the string: '" << dot_cc << "'" << std::endl;
    dot_h = dot_cc;
  }

  return strip_absolute_path(dot_h);
}

std::string TypeDescription::strip_absolute_path(const std::string &path)
{
  // prevent absolute paths from being stuffed in include paths
  // (that prevents installing a binary SCIRun with dynamic compilation)
  // do this by eliminating everything in front of the Core, Dataflow, or Packages
  std::string newpath = path;
  convertToUnixPath(newpath);
  size_t pos = newpath.find("Packages");
  if (pos == std::string::npos)
    pos = newpath.find("Core");
  if (pos == std::string::npos)
    pos = newpath.find("Dataflow");
  if (pos != std::string::npos)
    newpath = newpath.substr(pos, path.length()-pos);
  return newpath;
}

TypeDescription::Register::Register(const TypeDescription* /* td*/)
{
  // Registration happens in CTOR
}

TypeDescription::Register::~Register()
{
}

const TypeDescription* get_type_description(double*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("double", "builtin", "builtin");
  }
  return td;
}

const TypeDescription* get_type_description(std::complex<double>*)
{
  static TypeDescription* td = nullptr;
  if (!td){
    td = new TypeDescription("complex", "builtin", "builtin");
  }
  return td;
}

const TypeDescription* get_type_description(long*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("long", "builtin", "builtin");
  }
  return td;
}

const TypeDescription* get_type_description(long long*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("long long", "builtin", "builtin");
  }
  return td;
}

const TypeDescription* get_type_description(unsigned long long*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("unsigned long long", "builtin", "builtin");
  }
  return td;
}

const TypeDescription* get_type_description(float*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("float", "builtin", "builtin");
  }
  return td;
}

const TypeDescription* get_type_description(short*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("short", "builtin", "builtin");
  }
  return td;
}

const TypeDescription* get_type_description(unsigned short*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("unsigned short", "builtin", "builtin");
  }
  return td;
}

const TypeDescription* get_type_description(int*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("int", "builtin", "builtin");
  }
  return td;
}

const TypeDescription* get_type_description(unsigned int*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("unsigned int", "builtin", "builtin");
  }
  return td;
}

const TypeDescription* get_type_description(char*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("char", "builtin", "builtin");
  }
  return td;
}

const TypeDescription* get_type_description(unsigned char*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("unsigned char", "builtin", "builtin");
  }
  return td;
}

const TypeDescription* get_type_description(std::string*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("string", "std::string", "std");
  }
  return td;
}

const TypeDescription* get_type_description(unsigned long*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    td = new TypeDescription("unsigned long", "builtin", "builtin");
  }
  return td;
}

} // end namespace SCIRun
