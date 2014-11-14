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

#ifndef CORE_ALGORITHMS_FIELDS_REFINEMESH_REFINEMESHQUADSURFALGOV_H
#define CORE_ALGORITHMS_FIELDS_REFINEMESH_REFINEMESHQUADSURFALGOV_H 1

// Datatypes that the algorithm uses
#include <Core/Datatypes/DatatypeFwd.h> 
#include <Core/Datatypes/Legacy/Field/VMesh.h> 
#include <boost/unordered_map.hpp> 
// Base class for algorithm
#include <Core/Algorithms/Base/AlgorithmBase.h>

// for Windows support
#include <Core/Algorithms/Legacy/Fields/share.h>


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

struct edgepairhash
		{
				size_t operator()(const edgepair_t &a) const
				{
//#if defined(__ECC) || defined(_MSC_VER)
						//hash_compare<unsigned int> h;
//#else
					  //hash<unsigned int> h;
						//boost::hash <Key> h;  
						//HashKey h; 
//#endif
				//		return h((a.first<<3) ^ a.second);
				}
				static const size_t bucket_size = 4;
				static const size_t min_buckets = 8;
				bool operator()(const edgepair_t & a, const edgepair_t & b) const
				{
						return edgepairless::less(a,b);
				}
		};

class SCISHARE RefineMeshQuadSurfAlgoV : public AlgorithmBase
{
  public:  
    RefineMeshQuadSurfAlgoV();
	
	bool runImpl(FieldHandle input, FieldHandle& output) const; 
	bool runImpl(FieldHandle input, FieldHandle& output, std::string select, double isoval) const; 
	virtual AlgorithmOutput run_generic(const AlgorithmInput& input) const override; 
	
	typedef boost::unordered_map<edgepair_t, VMesh::Node::index_type, edgepairhash> edge_hash_type;

  private:

	VMesh::Node::index_type lookup(VMesh *refined,
                                 edge_hash_type &edgemap,
                                 VMesh::Node::index_type a,
                                 VMesh::Node::index_type b,
                                 double factor,
                                 std::vector<double>& ivalues);
								 
	Point RIinterpolate(VMesh *refined,
                      VMesh::Node::array_type& onodes,
                      double coords[2])
					    {
							Point result(0.0, 0.0, 0.0);
							
							double w[4];
							const double x = coords[0], y = coords[1];  
							w[0] = (-1 + x) * (-1 + y);
							w[1] = -x * (-1 + y);
							w[2] = x * y;
							w[3] = -(-1 + x) * y;

							Point p;
							for (int i = 0; i < 4; i++)
							{
							  refined->get_point(p, onodes[i]);
							  result += (p * w[i]);//.asVector();
							}
							return result;  
						}							 
	
	Double RIinterpolateV(std::vector<double>& ivalues,
                        VMesh::Node::array_type& onodes,
                        double coords[2])
						{
							double w[4];
							const double x = coords[0], y = coords[1];  
							w[0] = (-1 + x) * (-1 + y);
							w[1] = -x * (-1 + y);
							w[2] = x * y;
							w[3] = -(-1 + x) * y;

							return(w[0]*ivalues[onodes[0]] + w[1]*ivalues[onodes[1]] + 
								 w[2]*ivalues[onodes[2]] + w[3]*ivalues[onodes[3]]);
						}
						
						
	void dice(VMesh *refined, 
						 edge_hash_type &emap,
             VMesh::Node::array_type onodes,
             VMesh::index_type index, 
             VMesh::mask_type mask,
             VMesh::size_type maxnode,
             std::vector<double>& ivalues,
             std::vector<double>& evalues,
						 double vv,
						 int basis_order);


  VMesh::Node::index_type lookup(VMesh *refined,
                                 edge_hash_type &edgemap,
                                 VMesh::Node::index_type a,
                                 VMesh::Node::index_type b,
                                 double factor,
                                 std::vector<double>& ivalues)
  {
    edgepair_t ep;
    ep.first = a; ep.second = b;
    const edge_hash_type::iterator loc = edgemap.find(ep);
    if (loc == edgemap.end())
    {
      Point pa, pb;
      refined->get_point(pa, a);
      refined->get_point(pb, b);
      const Point inbetween = ((1.0-factor)*pa + (factor)*pb).asPoint();
      const VMesh::Node::index_type newnode = refined->add_point(inbetween);
      ivalues.push_back(((1.0-factor)*ivalues[a]+(factor)*ivalues[b]));
      edgemap[ep] = newnode;
      return newnode;
    }
    else
    {
      return (*loc).second;
    }
  }
			
};

								}}}}

#endif
