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

#ifndef CORE_ALGORTIHMS_FIELDS_MAPPING_MAPPING_DATA_SOURCE_H__
#define CORE_ALGORTIHMS_FIELDS_MAPPING_MAPPING_DATA_SOURCE_H__

#include <vector>
#include <Core/Algorithms/Legacy/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

// Class for extracting data from a field

class SCISHARE MappingDataSource
{
  public: 
    // Constructor
    MappingDataSource();
    virtual ~MappingDataSource();
    
    virtual void get_data(double& data, Point& p);
    virtual void get_data(Vector& data, Point& p);
    virtual void get_data(Tensor& data, Point& p);

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p);
    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p);
    virtual void get_data(std::vector<Tensor>& data, std::vector<Point>& p);
    
    bool is_double();
    bool is_scalar();
    bool is_vector();
    bool is_tensor();

  protected:
    bool is_double_;
    bool is_vector_;
    bool is_tensor_;
};

typedef LockingHandle<MappingDataSource> MappingDataSourceHandle;


// Function for creating it

bool SCISHARE CreateDataSource(MappingDataSourceHandle& handle, 
                      FieldHandle& sfield,
                      FieldHandle& wfield, 
                      AlgoBase* algo);


} // end namespace SCIRunAlgo

#endif