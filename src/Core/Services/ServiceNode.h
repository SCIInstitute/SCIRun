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


/* Core/ServiceNode.h 
 * 
 * auth: Jeroen Stinstra
 * adapted from: ComponentNode.h
 */

#ifndef JGS_CORE_SERVICES_SERVICENODE_H
#define JGS_CORE_SERVICES_SERVICENODE_H 1

#include <libxml/tree.h>
#include <libxml/parser.h>


#include <map>
#include <string>
#include <iostream>
#include <fstream>



namespace SCIRun {

// These functions are for maintaining services. A service is a thread that is started separately and 
// communicates with an external program. When a service port is assigned these services can be accessed
// over the internet as well. Since each package has different services, the service manager reads the 
// xml files in each package to see which services need to be started. This part of the code maintains
// the reading of these little pieces of XML code.

// Currently both the file only contains the serviced name and the object class that needs to be started to
// create the service.
// The format of the the xml files:
// e.g.
// <service name="matlabengine" class="MatlabInterface::Engine">
// <parameter>value</parameter>
// </service> 
//
// Unlike the Component service class, here everything is fitted in one structure
// There is not too much information to store anyway, so we do not bother about 
// optimizing memory use. Using one object makes the memory management way easier.

typedef struct {
  std::string servicename;
  std::string classname;
  std::string classpackagename;
  std::string	version;
  std::map<std::string,std::string> parameter;
} ServiceNode;


//////////////////////////
// ProcessServiceNode()
// Copies the contents of "tree" into the active fields of "node"

void ProcessServiceNode(const xmlNode* tree, ServiceNode& node);

//////////////////////////
// PrintServiceNode()
// Prints the contents of "node" to the screen.
// Used for debugging only.  fields with value NOT_SET
// are not printed.

void PrintServiceNode(ServiceNode& node);

//////////////////////////
// ReadServiceNodeFromFile()
// Parses "filename", which is in XML format, and then 
// copies the resulting DOM tree into "node"

int ReadServiceNodeFromFile(ServiceNode& node, const std::string filename);

} // End namespace SCIRun

#endif
