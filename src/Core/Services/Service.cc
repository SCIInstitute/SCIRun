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


// Service.cc

#include <Core/Services/Service.h>
#include <Core/Services/ServiceLog.h>
#include <Core/ICom/IComSocket.h>

namespace SCIRun {


Service::Service(const ServiceContext &ctx) :
	ctx_(ctx)
{
}

Service::~Service()
{
	// Close the socket if it was not closed yet
	// This function should be thread-safe

	ctx_.socket->close();

}


void Service::operator()()
{
	try
    {
        execute();
    }
    catch (...)
    {
        ctx_.socket->close();
        throw;
    }
	ctx_.socket->close();
}

void Service::execute()
{
}

void Service::errormsg(std::string error)
{
	ctx_.log->putmsg(std::string("Service (" + ctx_.servicename + std::string(") : error = ") + error));
}

void Service::warningmsg(std::string warning)
{
	ctx_.log->putmsg(std::string("Service (" + ctx_.servicename + std::string(") : warning = ") + warning));
}

bool Service::updateparameters()
{
	std::string filename = getparameter("fullrcfile");

  FILE* filein = fopen(filename.c_str(),"r");
  if (!filein) return (false);

  std::string read_buffer;
  size_t read_buffer_length = 512;
  read_buffer.resize(read_buffer_length);

  bool done = false;
  bool need_new_read = true;

  std::string linebuffer;
  size_t bytesread;
  size_t linestart;
  size_t lineend;
  size_t lineequal;
  size_t buffersize;
  size_t linedata;
  size_t linesize;
  size_t linetag;
  std::string tag;
  std::string data;

  while(!done)
	{
		bytesread = fread(&(read_buffer[0]),1,read_buffer_length,filein);
		if ((bytesread == 0)&&(!feof(filein)))
			{
				std::cerr << "Detected error while reading from file: " << filename << "\n";
				return(false);
			}
		if (bytesread > 0) linebuffer += read_buffer.substr(0,bytesread);

		if (feof(filein)) done = true;

		need_new_read = false;
		while (!need_new_read)
		{
			linestart = 0;
			buffersize = linebuffer.size();
			// Skip all newlines returns tabs and spaces at the start of a line
			while((linestart < buffersize)&&((linebuffer[linestart]=='\n')||(linebuffer[linestart]=='\r')||(linebuffer[linestart]=='\0')||(linebuffer[linestart]=='\t')||(linebuffer[linestart]==' '))) linestart++;

			std::string newline;
			// if bytesread is 0, it indicates an EOF, hence we just need to add the remainder
			// of what is in the buffer. The file has not properly terminated strings....
			if (bytesread == 0)
			{
				if(linestart < linebuffer.size()) newline = linebuffer.substr(linestart);
			}
			else
			{
				lineend = linestart;
				while((lineend < buffersize)&&(linebuffer[lineend]!='\n')&&(linebuffer[lineend]!='\r')&&(linebuffer[lineend]!='\0')) lineend++;
				if (lineend == linebuffer.size())
				{	// end of line not yet read
					need_new_read = true;
				}
				else
				{	// split of the latest line read
					newline = linebuffer.substr(linestart,(lineend-linestart));
					linebuffer = linebuffer.substr(lineend+1);
					need_new_read = false;
				}
			}

			if (!need_new_read)
			{
				if ((newline[0] == '#')||(newline[0] == '%'))
				{
					// Comment
				}
				else
				{
					lineequal = 0;
					linetag = 0;
					linesize = newline.size();
					while((linetag<linesize)&&((newline[linetag] != ' ')&&(newline[linetag] != '\t')&&(newline[linetag] != '='))) linetag++;
					tag = newline.substr(0,linetag);
					lineequal = linetag;
					while((lineequal < linesize)&&(newline[lineequal] != '=')) lineequal++;
					linedata = lineequal+1;
					while((linedata < linesize)&&((newline[linedata] == ' ')||(newline[linedata] == '\t'))) linedata++;
					data = newline.substr(linedata);
					ctx_.parameters[tag] = data;
				}
			}
		}
	}

  fclose(filein);

  return (true);
}


} // end namespace
