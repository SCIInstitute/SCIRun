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



/*
 *  BaseMC.h
 *
 *  Allen R. Sanderson
 *  SCI Institute
 *  University of Utah
 *  Feb 2008
 *
 */


#ifndef CORE_ALGORITHMS_VISUALIZATION_BASEMC_H
#define CORE_ALGORITHMS_VISUALIZATION_BASEMC_H 1

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <boost/unordered_map.hpp>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#include <Core/Geom/GeomObj.h>
#endif

#include <Core/Utils/Legacy/TypeDescription.h>


namespace SCIRun {

  class BaseMC
  {
  public:

    BaseMC() : build_field_(false), build_geom_(false), basis_order_(-1),
      nnodes_(0), ncells_(0) {}

    virtual ~BaseMC() {}

    virtual void reset(int,
      bool build_field,
      bool build_geom,
      bool transparency) = 0;

    virtual FieldHandle get_field(double val) = 0;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    GeomHandle   get_geom() { return geomHandle_; }
#endif

    Core::Datatypes::MatrixHandle get_interpolant();
    Core::Datatypes::MatrixHandle get_parent_cells();

    bool build_field_;
    bool build_geom_;
    int basis_order_;

    struct edgepair_t
    {
      SCIRun::index_type first;
      SCIRun::index_type second;
      double dfirst;
    };
  protected:
    struct edgepairhash
    {
      size_t operator()(const edgepair_t &a) const
      {
        boost::hash<size_t> h;
        return h((a.first << 3) ^ a.second);
      }
    };

    typedef boost::unordered_map<edgepair_t, SCIRun::index_type, edgepairhash> edge_hash_type;

    std::vector<SCIRun::index_type> cell_map_;  // Unique cells when surfacing node data.
    std::vector<SCIRun::index_type> node_map_;  // Unique nodes when surfacing cell data.

    SCIRun::size_type nnodes_;
    SCIRun::size_type ncells_;

    edge_hash_type edge_map_;  // Unique edge cuts when surfacing node data
  };

  inline bool operator==(const BaseMC::edgepair_t& lhs, const BaseMC::edgepair_t& rhs)
  {
    return lhs.first == rhs.first && lhs.second == rhs.second;
  }

  inline bool operator!=(const BaseMC::edgepair_t& lhs, const BaseMC::edgepair_t& rhs)
  {
    return !(lhs == rhs);
  }

} // End namespace SCIRun

#endif
