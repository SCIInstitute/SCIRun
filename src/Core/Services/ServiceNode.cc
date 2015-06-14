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

/* Core/ServiceNode.cc 
 * 
 * auth: Jeroen Stinstra
 * adapted from: ComponentNode.cc
*/

#include <Core/Services/ServiceNode.h>
#include <Core/XMLUtil/XMLUtil.h>

#include <boost/algorithm/string.hpp>

#include <stdlib.h>


namespace SCIRun {

void ProcessServiceNode(const xmlNodePtr d, ServiceNode& n)
{

  // Get the service name:
  // This name needs to be unique and is used when connecting to services
	
  xmlAttrPtr name = get_attribute_by_name(d, "name");
  if (name == 0) 
  {
    std::cout << "ERROR: Service has no name." << std::endl;
  }
  else 
  {
    n.servicename = std::string(to_char_ptr(name->children->content));
  }

  // Get service class name. This is used to launch and load the service
  // when needed. Only when the service is requested it is launched.

  xmlAttrPtr classname = get_attribute_by_name(d, "class");
  if (classname == 0) 
  {
    std::cout << "ERROR: Service " << n.servicename << " has no class object." 
	      << std::endl;
  }
  else 
  {
    n.classname = std::string(to_char_ptr(classname->children->content));
  }

  // Get service classpackage name. This specifies the location of the 
  // dynamic file to run to provide this service. If this one is not 
  // supplied it is assumed to in the same directory as the class it self
  xmlAttrPtr classpackagename = get_attribute_by_name(d, "classpackage");
  if (classpackagename != 0) 
  {
    n.classpackagename = 
      std::string(to_char_ptr(classpackagename->children->content));
  }

  // Get service version number. This is used to keep track of bugs and changes
  xmlAttrPtr versionname = get_attribute_by_name(d, "version");
  if (versionname == 0) 
  {
    std::cout << "ERROR: Service " << n.servicename <<" has no version number" << std::endl;
  }
  else 
  {
    n.version = std::string(to_char_ptr(versionname->children->content));
  }

  // Go through the parameter list and load parameters into ServiceNode
	
  for (xmlNode *child = d->children; child != 0; child = child->next)
  {
    std::string pname = std::string(to_char_ptr(child->name));
    if (pname == "#text") continue;
    std::string sc = get_serialized_children(child);
	  boost::trim(sc);
    n.parameter[pname] = sc;
  }
}


int ReadServiceNodeFromFile(ServiceNode& n, const std::string filename)
{
  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION;

    
  xmlDoc *doc = xmlReadFile(filename.c_str(), NULL, 0);
  if (doc == 0) {
    std::cerr << "ServiceNode.cc: Failed to parse " << filename << std::endl;
    return 0;
  }

  xmlNode* node = doc->children;
  for (; node != 0; node = node->next) {
    if (node->type == XML_ELEMENT_NODE && 
	std::string(to_char_ptr(node->name)) == std::string("service")) 
    {
      ProcessServiceNode(node, n);
    }
  }

  xmlFreeDoc(doc);
#ifndef _WIN32
  // there is some problem in the windows version which is 
  // either caused or exploited by this - LIBXML_TEST_VERSION
  // will fail when called from a different module
  xmlCleanupParser();
#endif

  return 1;
}

} // End namespace SCIRun


