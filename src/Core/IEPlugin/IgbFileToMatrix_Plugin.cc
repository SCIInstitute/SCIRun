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

/*
 *  IgbFiletoMatrix_Plugin.cc
 *
 *  Written by:
 *   Karli Gillette
 *   Department of Bioengineering
 *   University of Utah
 *
 */
 

#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Core/IEPlugin/IgbFileToMatrix_Plugin.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Logging/LoggerInterface.h>
#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <string.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <cmath>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Datatypes;

MatrixHandle SCIRun::IgbFileMatrix_reader(LoggerHandle pr, const char *filename)
{
     
    string line;
    vector<string> strs;
    int x_size=0;
    int t_size=0;
    int count=0;
   
    
    ifstream myfile (filename);
    if (myfile.is_open())
    {
        //while ( myfile.good() )
        for(int i=0; i<5; i++)
        {
            getline (myfile,line);
            
            boost::split(strs,line,boost::is_any_of(":, "));
            size_t sz;
            sz=line.size();
            
            
            
            for(int p=0;p<sz;p++)
            {
             
               std::cout << p << std::endl; 	
             
                if (boost::iequals(strs[p], "x"))
                {
                    x_size=atoi(strs[p+1].c_str());
                
                    
                    count += 1;
                    
                }
                if (boost::iequals(strs[p], "t"))
                {
                    t_size=atoi(strs[p+1].c_str());
                
                    
                    count +=1;
                     
                }
                
                if (count ==2 ) break;
                
  
            }
        
                     
        	}
        myfile.close();
    }

            
        streamoff length=0;
        streamsize numbyts=0;
        //char * buffer;
        
        
        
        
        ifstream is;
        is.open (filename, ios::in |  ios::binary );
        if (is.is_open())
        {
            // get length of file:
            is.seekg (0, ios::end);
            length = is.tellg();
            is.seekg (1024, ios::beg);

            vector<float> vec;
            vec.resize(x_size*t_size);
            
            
        
            is.read ((char*)&vec[0],x_size * t_size * sizeof(float));
            if (!is)
            {
                numbyts=is.gcount();
                cout << "Error reading binary data. Number of bytes read: " << numbyts << endl;

            }
            is.close();
        
        
            // auto result(boost::make_shared<DenseMatrix>(x_size,t_size));
            
            DenseMatrixHandle result;
            
            result.reset(new DenseMatrix(x_size,t_size));
            
                for(size_t p=0;p<t_size;p++ )
                {
                
                    for(size_t pp=0;pp<x_size;pp++ )
                    {
            
                	
                        (*result)(pp, p) = vec[(p*x_size)+pp];
                		
                    }
                }
           
    
        return(result);
        }
}
  
  
  




