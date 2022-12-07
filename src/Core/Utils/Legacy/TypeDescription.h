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


///    @file    TypeDescription.h
///    @author  Martin Cole
///    @date    Mon May 14 10:16:28 2001

#if ! defined(Disclosure_TypeDescription_h)
#define Disclosure_TypeDescription_h

#include <vector>
#include <string>
#include <complex>

#include <Core/Utils/Legacy/share.h>

namespace SCIRun {

struct CompileInfo;

class SCISHARE TypeDescription {
public:
  enum category_e {
    DATA_E,
    BASIS_E,
    MESH_E,
    CONTAINER_E,
    FIELD_E,
    OTHER_E
  };

  typedef std::vector<const TypeDescription*> td_vec;

  TypeDescription(const std::string& name,
		  const std::string& path,
		  const std::string& namesp,
		  category_e c = OTHER_E);
  TypeDescription(const std::string& name,
		  td_vec *sub, // this takes ownership of the memory.
		  const std::string& path,
		  const std::string& namesp,
		  category_e c = OTHER_E);
  ~TypeDescription();

  td_vec* get_sub_type() const {
    return subtype_;
  }
  /// The arguments determine how the templated types are separated.
  /// default is "<" and "> "
  std::string get_name(const std::string & type_sep_start = "<",
		  const std::string & type_sep_end = "> ") const;
  std::string get_similar_name(const std::string &substitute,
			  const int pos,
			  const std::string & type_sep_start = "<",
			  const std::string & type_sep_end = "> ") const;

  std::string get_filename() const;

  std::string get_h_file_path() const { return h_file_path_; }
  std::string get_namespace() const { return namespace_; }

  struct Register {
    explicit Register(const TypeDescription*);
    ~Register();
  };

  /// convert a string that ends in .cc to end in .h
  static std::string cc_to_h(const std::string &dot_cc);
  static std::string strip_absolute_path(const std::string &path);
  static const TypeDescription* lookup_type(const std::string&);

private:
  td_vec                     *subtype_;
  std::string                name_;
  std::string                h_file_path_;
  std::string                namespace_;
  category_e                 category_;
  // Hide these methods
  TypeDescription(const TypeDescription&);
  TypeDescription& operator=(const TypeDescription&);

  void register_type();
};


SCISHARE const TypeDescription* get_type_description(double*);
SCISHARE const TypeDescription* get_type_description(std::complex<double>*);
SCISHARE const TypeDescription* get_type_description(float*);
SCISHARE const TypeDescription* get_type_description(short*);
SCISHARE const TypeDescription* get_type_description(unsigned short*);
SCISHARE const TypeDescription* get_type_description(int*);
SCISHARE const TypeDescription* get_type_description(unsigned int*);
SCISHARE const TypeDescription* get_type_description(long*);
SCISHARE const TypeDescription* get_type_description(unsigned long*);
SCISHARE const TypeDescription* get_type_description(long long*);
SCISHARE const TypeDescription* get_type_description(unsigned long long*);
SCISHARE const TypeDescription* get_type_description(char*);
SCISHARE const TypeDescription* get_type_description(unsigned char*);
SCISHARE const TypeDescription* get_type_description(bool*);
SCISHARE const TypeDescription* get_type_description(std::string*);

template <class T>
const TypeDescription* get_type_description(std::vector<T>*)
{
  static TypeDescription* td = nullptr;
  if(!td){
    const TypeDescription *sub = get_type_description(static_cast<T*>(nullptr));
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("vector", subs, "std::vector", "std",
				TypeDescription::CONTAINER_E);
  }
  return td;
}

template <class T1, class T2>
const TypeDescription* get_type_description (std::pair<T1,T2> *)
{
  static TypeDescription* td = nullptr;
  if(!td){
    const TypeDescription *sub1 = get_type_description(static_cast<T1*>(nullptr));
    const TypeDescription *sub2 = get_type_description(static_cast<T2*>(nullptr));
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(2);
    (*subs)[0] = sub1;
    (*subs)[1] = sub2;
    td = new TypeDescription("pair", subs, "std::utility", "std",
				TypeDescription::CONTAINER_E);
  }
  return td;

}

} // End namespace SCIRun

#endif //Disclosure_TypeDescription_h
