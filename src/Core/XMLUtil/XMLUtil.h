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


#ifndef Dataflow_XMLUtil_XMLUtil_H
#define Dataflow_XMLUtil_XMLUtil_H

#include <libxml/tree.h>
#include <libxml/parser.h>

#include <string>
#include <iosfwd>
#include <vector>

#include <Core/XMLUtil/share.h>

namespace SCIRun {

inline
const char* to_char_ptr(const xmlChar *t) {
  return (const char*)t;
}

inline
const xmlChar* to_xml_ch_ptr(const char *t) {
  return (xmlChar*)t;
}

inline
bool string_is(const xmlChar *childname, const char *const name) {
  return (xmlStrcmp(childname, xmlCharStrdup(name)) == 0);
}

SCISHARE xmlAttrPtr get_attribute_by_name(const xmlNodePtr p, const char *name);
SCISHARE bool get_attributes(std::vector<xmlNodePtr> &att, const xmlNodePtr p);


//////////////////////////////
// getSerializedChildren()
// returns a string in XML format that
// represents the children of the node
// named "node".

SCISHARE std::string get_serialized_children(xmlNode* node);



  namespace XMLUtil {

    inline const char* xmlChar_to_char(const xmlChar *xmlchar) {
      return (const char *)xmlchar;
    }

    inline const xmlChar* char_to_xmlChar(const char *chrs) {
      return (const xmlChar *)chrs;
    }


    inline std::string xmlChar_to_string(const xmlChar *xmlchar) {
      if (xmlchar)
        return std::string(xmlChar_to_char(xmlchar));
      else
        return std::string("");
    }

    SCISHARE bool node_is_element(const xmlNodePtr p, const char *);
    SCISHARE bool node_is_dtd(const xmlNodePtr p, const std::string &);
    SCISHARE bool node_is_comment(const xmlNodePtr p);

    SCISHARE const char *maybe_get_att_as_const_char_str(const xmlNodePtr p, const char *);
    SCISHARE bool maybe_get_att_as_int(const xmlNodePtr p, const std::string &,
                                       int &);
    SCISHARE bool maybe_get_att_as_double(const xmlNodePtr p, const std::string &,
                                          double&);
    SCISHARE bool maybe_get_att_as_string(const xmlNodePtr p, const std::string &,
                                          std::string&);


    SCISHARE int node_att_as_int(const xmlNodePtr p, const std::string &);
    SCISHARE double node_att_as_double(const xmlNodePtr p, const std::string &);
    SCISHARE std::string node_att_as_string(const xmlNodePtr p, const std::string &);
    SCISHARE const char * node_att_as_const_char_str(const xmlNodePtr p, const char *);
  }


} // End namespace SCIRun

#endif
