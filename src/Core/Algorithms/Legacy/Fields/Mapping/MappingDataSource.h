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


#ifndef CORE_ALGORTIHMS_FIELDS_MAPPING_MAPPING_DATA_SOURCE_H__
#define CORE_ALGORTIHMS_FIELDS_MAPPING_MAPPING_DATA_SOURCE_H__

#include <vector>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <Core/Thread/Interruptible.h>
#include <Core/Algorithms/Legacy/Fields/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {
      namespace Fields {

// Class for extracting data from a field

class SCISHARE MappingDataSource : public Thread::Interruptible
{
  public:
    MappingDataSource();
    virtual ~MappingDataSource();

    virtual void get_data(double& data, const Geometry::Point& p) const;
    virtual void get_data(Geometry::Vector& data, const Geometry::Point& p) const;
    virtual void get_data(Geometry::Tensor& data, const Geometry::Point& p) const;

    virtual void get_data(std::vector<double>& data, const std::vector<Geometry::Point>& p) const;
    virtual void get_data(std::vector<Geometry::Vector>& data, const std::vector<Geometry::Point>& p) const;
    virtual void get_data(std::vector<Geometry::Tensor>& data, const std::vector<Geometry::Point>& p) const;

    bool is_double() const;
    bool is_scalar() const;
    bool is_vector() const;
    bool is_tensor() const;

  protected:
    bool is_double_;
    bool is_vector_;
    bool is_tensor_;
};

typedef boost::shared_ptr<MappingDataSource> MappingDataSourceHandle;

MappingDataSourceHandle SCISHARE CreateDataSource(FieldHandle sfield, FieldHandle wfield, const AlgorithmBase* algo);

}}}}

#endif
