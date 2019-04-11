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

#include <Core/Algorithms/Fields/FieldData/SmoothVecFieldMedian.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Geometry/Vector.h>
#include <Core/Math/MiscMath.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
SmoothVecFieldMedianAlgo::run(FieldHandle input, FieldHandle& output)
{
  algo_start("SmoothVecFieldMedian");
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }
  
  FieldInformation fi(input);

  if (fi.is_nodata())
  {
    error("Input field does not have data associated with it");
    algo_end(); return (false);    
  }

  if (!(fi.is_vector()))
  {
    error("The data needs to be of vector type");
    algo_end(); return (false);    
  }

  output = CreateField(fi,input->mesh());

  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);      
  }

  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  VMesh* imesh = input->vmesh();
  VMesh::Elem::index_type a,b;
  Vector v0,v1,v2,v3;  
  std::vector<double> angles, original;
  int middle=0, not_on_list=0, myloc=0;
  double gdot=0, m1=0, m2=0, angle=0;
   
  
  

  if (ifield->is_vector())
  {
    VField::size_type num_values = ifield->num_values();
  
    imesh->synchronize(Mesh::ELEM_NEIGHBORS_E);

   
    
    
  
    int cnt = 0;
   // for (VField::index_type idx = 0; idx < 2000; idx++)
    for (VMesh::Elem::index_type idx = 0; idx < num_values; idx++)
    {
      //calculate neighborhoods
    VMesh::Elem::array_type nci, ncitot, Nlist;
    VMesh::Elem::array_type nci2;
      //cerr<<"one loop"<<idx<<endl;
      imesh->get_neighbors(nci, idx);
      
      ncitot.push_back(idx);
      
      for (size_t t=0; t< nci.size(); t++)
      {
         ncitot.push_back(nci[t]);
         imesh->get_neighbors(nci2, nci[t]);
         
         for (size_t L=0; L< nci2.size(); L++)
         {            
           ncitot.push_back(nci2[L]);
         } 
         
         for (size_t t2=0; t2< nci2.size(); t2++)
         {
           VMesh::Elem::array_type nci3;

           imesh->get_neighbors(nci3, nci2[t2]);
           for (size_t L=0; L< nci3.size(); L++)
           {            
             ncitot.push_back(nci3[L]);
           }         
         }
      }
      
      not_on_list=0;
      for (size_t p1=0; p1< ncitot.size(); p1++)
      {
        not_on_list=0;
        for (size_t p2=0; p2< Nlist.size(); p2++)  
        {  
          if(ncitot[p1]==Nlist[p2])
          {
            not_on_list=1;
          }
          
        }
        if(not_on_list==0)
        {
          Nlist.push_back(ncitot[p1]);
        }
      }
            
      angles.clear();
      original.clear();
      ifield->get_value(v0,idx);
      for (size_t q=0; q< Nlist.size(); q++)
      {
        a=Nlist[q];
        ifield->get_value(v1,a);
        if(v0.length()*v1.length()==0)
        {
          angles.push_back(0);
          original.push_back(0);
        }
        else
        {
          gdot=Dot(v0,v1);
          m1=v0.length();
          m2=v1.length();
          angle=(gdot/(m1*m2));
          angles.push_back(angle);
          original.push_back(angle);
        }
      } 
      
      sort(angles.begin(), angles.end());
      middle=(int)((angles.size()+1)/2);
      for(size_t k=0; k<original.size(); k++)
      {
        if(original[k]==angles[middle])
        {
          myloc=k;
          k=original.size();
        }
      }

      b=Nlist[myloc];
      ifield->get_value(v2,b);
           
      ofield->set_value(v2,idx);
      
      cnt++; if (cnt == 200) { cnt = 0; update_progress(idx,num_values); }
    }
  }
  algo_end(); return (true);
}

} // end namespace SCIRun
