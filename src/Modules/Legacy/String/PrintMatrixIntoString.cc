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

#include <stdio.h>
#include <Dataflow/Network/Module.h>

#include <Core/Datatypes/String.h>
#include <Dataflow/Network/Ports/StringPort.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseColMajMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif

namespace SCIRun {

/// @class PrintMatrixIntoString
/// @brief This module does a sprintf with input matrices into a new string. 

class PrintMatrixIntoString : public Module {
  public:
    PrintMatrixIntoString(GuiContext*);
    virtual ~PrintMatrixIntoString() {}
    virtual void execute();

  private:
    GuiString formatstring_;
};


DECLARE_MAKER(PrintMatrixIntoString)
PrintMatrixIntoString::PrintMatrixIntoString(GuiContext* ctx)
  : Module("PrintMatrixIntoString", ctx, Source, "String", "SCIRun"),
    formatstring_(get_ctx()->subVar("formatstring"), "time: %5.4f ms")
{
}


void
PrintMatrixIntoString::execute()
{
  std::string   format, output;
  
  MatrixHandle currentmatrix = 0;
  int inputport = 0;
  index_type matrixindex = 0;
  double       datavalue = 0;
  double*      dataptr = 0;
  bool         lastport = false;
  bool         lastdata = false;
  bool         isformat = false;
  
  std::vector<char> buffer(256);  
  
  format = formatstring_.get();

  StringHandle  stringH;
  if (get_input_handle("Format", stringH, false))
  {
    format = stringH->get();
  }

  // Get the dynamic handles
  std::vector<MatrixHandle> matrixH;
  get_dynamic_input_handles("Input",matrixH,false);

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
            &&(format[j] != 'F')&&(format[j] != 'A')&&(format[j] != 'a')&&(format[j] != 'p')&&(format[j] != 'P')) j++;
    
        if (j == format.size())
        {
            error("Improper format string '%..type' clause was incomplete");
            return;
        }
              
        std::string fstr = format.substr(i,j-i+1);
        
        if ((format[j] != 's')&&(format[j] != 'S')&&(format[j] != 'C')&&(format[j] != 'c')&&(format[j] != 'p')&&(format[j] != 'P'))
        {
          isformat  = true;
          datavalue = 0.0;
          while ((currentmatrix.get_rep() == 0)&&(lastport==false))
          {
            if (static_cast<size_t>(inputport) >= matrixH.size())
            {
              lastport = true;
              lastdata = true;
            }
            else
            {
              currentmatrix = matrixH[inputport]; 
              inputport++;
              matrixindex = 0;
              if (currentmatrix.get_rep())
              {
                if (currentmatrix->get_data_size() == 0) currentmatrix = 0;
              }
              
              if (currentmatrix.get_rep())
              {
                // Check whether we need to transpose matrix
                // If so we transpose the whole matrix
                if (matrix_is::dense_col_maj(currentmatrix))
                {
                  currentmatrix = currentmatrix->dense();
                }
              }
            }
          }
          
          if (currentmatrix.get_rep())
          {
            dataptr = currentmatrix->get_data_pointer();
            if (matrixindex < currentmatrix->get_data_size())
            {
              datavalue = dataptr[matrixindex]; matrixindex++;
            }
            else
            {
              datavalue = 0.0;
            }
            if (matrixindex == currentmatrix->get_data_size()) 
            { 
              currentmatrix = 0; 
              if (static_cast<size_t>(inputport) == matrixH.size())
              {
                lastdata = true;
                lastport = true;
              }
            }
          }
        }
        
        if ((format[j] == 's')||(format[j] == 'S')||(format[j] == 'c')||(format[j] == 'C'))
        {
          // We put the %s %S back in the string so it can be filled out lateron
          // By a different module
          output += fstr;
          i = j+1;
        }
        else if ((format[j] == 'd')||(format[j] == 'o'))
        {
          int scalar = static_cast<int>(datavalue);
          snprintf(&(buffer[0]),256,fstr.c_str(),scalar);
          output += std::string(reinterpret_cast<char *>(&(buffer[0])));
          i = j+1;
        }
        else if ((format[j] == 'i')||(format[j] == 'u')||(format[j] == 'x')||(format[j] == 'X'))
        {
          unsigned int scalar = static_cast<unsigned int>(datavalue);
          snprintf(&(buffer[0]),256,fstr.c_str(),scalar);
          output += std::string(reinterpret_cast<char *>(&(buffer[0])));
          i = j+1;                
        }
        else if ((format[j] == 'e')||(format[j] == 'E')||(format[j] == 'f')||(format[j] == 'F')||
                 (format[j] == 'g')||(format[j] == 'G')||(format[j] == 'a')||(format[j] == 'A'))
        {
          snprintf(&(buffer[0]),256,fstr.c_str(),datavalue);
          output += std::string(reinterpret_cast<char *>(&(buffer[0])));
          i = j+1;   
        }
        else if ((format[j] == 'p')||(format[j] == 'P'))
        {
          fstr[fstr.size()-1] = 'g';
          double ptime = get_time();
          snprintf(&(buffer[0]),256,fstr.c_str(),ptime);
          output += std::string(reinterpret_cast<char *>(&(buffer[0])));
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
      output += format[i]; i++;
    }
    
    if ((i== format.size())&&(isformat == true)&&(lastdata == false))
    {
      i = 0;
    }
  }

  StringHandle handle(new String(output));
  send_output_handle("Output", handle);
}

} // End namespace SCIRun


