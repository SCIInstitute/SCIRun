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

#ifndef CORE_ALGORITHMS_FIELDS_REFINEMESH_REFINEMESHHEXVOLALGOV_H
#define CORE_ALGORITHMS_FIELDS_REFINEMESH_REFINEMESHHEXVOLALGOV_H 1

// Datatypes that the algorithm uses
#include <Core/Datatypes/DatatypeFwd.h> 
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h> 
// Base class for algorithm
#include <Core/Algorithms/Base/AlgorithmBase.h>

// for Windows support
#include <Core/Algorithms/Legacy/Fields/share.h>
#include <boost/unordered_map.hpp> 

using namespace SCIRun::Core::Geometry;
//using namespace SCIRun::Core::Algorithms::Fields; 

namespace SCIRun{
		namespace Core{
				namespace Algorithms{
						namespace Fields{

struct edgepair_t
    {
      VMesh::index_type first;
      VMesh::index_type second;
    };

struct edgepairequal
    {
      bool operator()(const edgepair_t &a, const edgepair_t &b) const
      {
        return a.first == b.first && a.second == b.second;
      }
    };

struct edgepairless
    {
      bool operator()(const edgepair_t &a, const edgepair_t &b)
      {
        return less(a, b);
      }
      static bool less(const edgepair_t &a, const edgepair_t &b)
      {
        return a.first < b.first || a.first == b.first && a.second < b.second;
      }
    };

struct IndexHash {
  static const size_t bucket_size = 4;
  static const size_t min_buckets = 8;
  
  size_t operator()(const index_type &idx) const
    { return (static_cast<size_t>(idx)); }
  
  bool operator()(const index_type &i1, const index_type &i2) const
    { return (i1 < i2); }
};

typedef boost::unordered_map<index_type,index_type,IndexHash> hash_map_type;



class SCISHARE RefineMeshHexVolAlgoV : public AlgorithmBase
{
  public:  
    RefineMeshHexVolAlgoV();
      
		bool runImpl(FieldHandle input, FieldHandle& output, std::string select, double isoval) const; 
		//bool runImpl(FieldHandle input, FieldHandle& output) const; 

		virtual AlgorithmOutput run_generic(const AlgorithmInput& input) const override; 
		
protected:
		
 private:
  
		void RefineMeshHexVolAlgoV::set_table(int i, int pattern, int reorder);
		void RefineMeshHexVolAlgoV::init_pattern_table();
		
    int pattern_table[256][2];
		SCIRun::Core::Geometry::Point hcoords[8];
    
    static int hex_reorder_table[14][8];
    static double hcoords_double[8][3];
    
    VMesh::index_type maxnode;
		VMesh::Node::index_type add_point(VMesh *refined,
                                      VMesh::Node::array_type &nodes,
                                      const int *reorder, 
                                      unsigned int a, 
                                      unsigned int b,
                                      double factor,
                                      std::vector<double>& ivalues,
                                      int basis_order);
		VMesh::Node::index_type add_point_convex(VMesh *refined,
                                      VMesh::Node::array_type &nodes,
                                      const int *reorder, 
                                      VMesh::index_type a, 
                                      VMesh::index_type b,
                                      std::vector<double>& ivalues,
                                      int basis_order);
	  VMesh::Node::index_type add_point_convex(VMesh *refined,
                                      VMesh::Node::array_type &nodes,
                                      Point coordsp,
                                      std::vector<double>& ivalues,
                                      int basis_order);
		void dice(VMesh *refined,
                           hash_map_type &emap,
                           VMesh::Node::array_type nodes,
                           VMesh::index_type index,
                           VMesh::mask_type mask,
                           std::vector<double>& ivalues,
                           std::vector<double>& evalues,
                           double vv,
                           int basis_order);
		 VMesh::Node::index_type lookup(VMesh *refined,
                                   hash_map_type &edgemap,
                                   VMesh::Node::array_type &nodes,
                                   const int *reorder, 
                                   VMesh::index_type a, 
                                   VMesh::index_type b,
                                   double factor,
                                   std::vector<double>& ivalues,
                                   int basis_order);
	};
						}}}}

#endif
