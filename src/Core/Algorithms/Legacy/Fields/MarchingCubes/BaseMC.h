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

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 #include <sci_defs/hashmap_defs.h>
#endif

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

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

    virtual void reset( int,
            bool build_field,
            bool build_geom,
            bool transparency ) = 0;
            
    virtual FieldHandle get_field(double val) = 0;
    
    #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
     GeomHandle   get_geom() { return geomHandle_; }
    #endif
    
    Core::Datatypes::MatrixHandle get_interpolant();     
    Core::Datatypes::MatrixHandle get_parent_cells();
    
    bool build_field_;
    bool build_geom_;
    int basis_order_;
 
  protected:
  
  struct edgepair_t
  {
    SCIRun::index_type first;
    SCIRun::index_type second;
    double dfirst;
  };
  
  struct edgepairless
  {
    bool operator()(const edgepair_t &a, const edgepair_t &b) const
    {
      return less(a,b);
    }
    static bool less(const edgepair_t &a, const edgepair_t &b)
    {
      return a.first < b.first || (a.first == b.first && a.second < b.second);
    }
  }; 

    std::vector<SCIRun::index_type> cell_map_;  // Unique cells when surfacing node data.
    std::vector<SCIRun::index_type> node_map_;  // Unique nodes when surfacing cell data.

    SCIRun::size_type nnodes_;
    SCIRun::size_type ncells_;
  
    //GeomHandle geomHandle_;
    
    #ifdef HAVE_HASH_MAP
    
    struct edgepairequal
    {
      bool operator()(const edgepair_t &a, const edgepair_t &b) const
      {
        return a.first == b.first && a.second == b.second;
      }
    };

    struct edgepairhash
    {
      unsigned int operator()(const edgepair_t &a) const
      {
  #if defined(__ECC) || defined(_MSC_VER)
        hash_compare<unsigned int> h;
  #else
        hash<unsigned int> h;
  #endif
        return h(a.first ^ a.second);
      }
  # if defined(__ECC) || defined(_MSC_VER)

        // These are particularly needed by ICC's hash stuff
        static const size_t bucket_size = 4;
        static const size_t min_buckets = 8;
        
        // This is a less than function.
        bool operator()(const edgepair_t & a, const edgepair_t & b) const {
          return edgepairless::less(a,b);
        }
  # endif // endif ifdef __ICC
    };
  
  # if defined(__ECC) || defined(_MSC_VER)
    typedef hash_map<edgepair_t, SCIRun::index_type, edgepairhash> edge_hash_type;
  #else
    typedef hash_map<edgepair_t,
         SCIRun::index_type,
         edgepairhash,
         edgepairequal> edge_hash_type;
  #endif // !defined(__ECC) && !defined(_MSC_VER)
    
  #else
    typedef std::map<edgepair_t,SCIRun::index_type,edgepairless> edge_hash_type;
  #endif
    edge_hash_type edge_map_;  // Unique edge cuts when surfacing node data
};
     
} // End namespace SCIRun

#endif 
