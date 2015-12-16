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

#include <Modules/Legacy/String/PrintStringIntoString.h>
#include <stdio.h>

#include <Core/Datatypes/String.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif


using namespace SCIRun::Modules::StringManip;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

SCIRun::Core::Algorithms::AlgorithmParameterName PrintStringIntoString::FormatString("FormatString");

const ModuleLookupInfo PrintStringIntoString::staticInfo_("PrintStringIntoString", "String", "SCIRun");

PrintStringIntoString::PrintStringIntoString() : Module(staticInfo_)
{
  INITIALIZE_PORT(Format);
  INITIALIZE_PORT(Input);
  INITIALIZE_PORT(Output);
}

/// @class PrintStringIntoString
/// @brief This module does a sprintf with input strings into a new string. 


void PrintStringIntoString::setStateDefaults()
{
  auto state = get_state();
  state->setValue(FormatString,std::string ("my string: %s"));
}

void
PrintStringIntoString::execute()
{
  std::string   format, output;
  
  StringHandle currentstring;
  int          inputport = 0;  
  std::string  str;
  
  std::vector<char> buffer(256);
  bool    lastport = false;
  
  
  auto  stringH = getOptionalInput(Format);
  
  auto state = get_state();
  
  // check for port input and in none use gui input
  if (stringH && *stringH)
  {
    state -> setValue(FormatString, (*stringH) -> value());
  }
  format = state -> getValue(FormatString).toString();
  
  
  // Get the dynamic handles
  auto stringsH = getOptionalDynamicInputs(Input);

  
  if (needToExecute())
  {
    size_t i = 0;
    while(i < format.size())
    {
      if (format[i] == '%')
      {
        if (i == format.size()-1)
        {
          error("Improper format string '%' is last character");
          return;
      }
              
        if (format[i+1] == '%')
        {
            output += '%'; i += 2;
        }
        else
        {
          size_t j = i+1;
          // Just to name a few printing options
          while((j < format.size())&&(format[j] != 'd')&&(format[j] != 'e')&&(format[j] != 'g')&&(format[j] != 'c')
              &&(format[j] != 'i')&&(format[j] != 'E')&&(format[j] != 'x')&&(format[j] != 'X')&&(format[j] != 's')
              &&(format[j] != 'u')&&(format[j] != 'o')&&(format[j] != 'g')&&(format[j] != 'G')&&(format[j] != 'f')
              &&(format[j] != 'F')&&(format[j] != 'A')&&(format[j] != 'a')&&(format[j] != 's')&&(format[j] != 'C')&&(format[j] != 'p')&&(format[j] != 'P')) j++;
      
          if (j == format.size())
          {
              error("Improper format string '%..type' clause was incomplete");
              return;
          }
                
          std::string fstr = format.substr(i,j-i+1);
          
          {
            str = "";
            if (lastport == false)
            {
              if (inputport == stringsH.size())
              {
                lastport = true;
              }
              else
              {
                if (stringsH.size() == static_cast<size_t>(inputport))
                {
                  lastport = true;
                }
                else
                {
                  currentstring = stringsH[inputport]; inputport++;
                  if (currentstring)
                  {
                    str = currentstring->value();
                  }
                }
              }
            }
          }
          
          if ((format[j] == 's')||(format[j] == 'S')||(format[j] == 'c')||(format[j] == 'C'))
          {
            // We put the %s %S back in the string so it can be filled out lateron
            // By a different module
            
            if (j == i+1)
            {
              output += str;
            }
            else
            {   
              // there is some modifier or so
              // This implementation is naive in assuming only
              // a buffer of 256 bytes. This needs to be altered one
              // day.
              snprintf(&(buffer[0]),256,fstr.c_str(),str.c_str());
              output += std::string(static_cast<char *>(&(buffer[0])));
            }
            i = j+1;
          
          }
          else if ((format[j] == 'd')||(format[j] == 'o')||(format[j] == 'i')||
                  (format[j] == 'u')||(format[j] == 'x')||(format[j] == 'X')||
                  (format[j] == 'e')||(format[j] == 'E')||(format[j] == 'f')||
                  (format[j] == 'F')||(format[j] == 'g')||(format[j] == 'G')||
                  (format[j] == 'a')||(format[j] == 'A')||(format[j] == 'p')||
                  (format[j] == 'P'))
          {
            output += fstr;
            i = j+1;
          }
        }
      }
      else if ( format[i] == '\\')
      {
        if (i < (format.size()-1))
        {
          switch (format[i+1])
          {
            case 'n': output += '\n'; break;
            case 'b': output += '\b'; break;
            case 't': output += '\t'; break;
            case 'r': output += '\r'; break;
            case '\\': output += '\\'; break;
            case '0': output += '\0'; break;
            case 'v': output += '\v'; break;
            default:
              error("unknown escape character");
              return;
          }
          i = i+2;
        }
      }
      else
      {
        output += format[i++];
      }
    }

    StringHandle handle(new String(output));
    sendOutput(Output, handle);
  }
}


