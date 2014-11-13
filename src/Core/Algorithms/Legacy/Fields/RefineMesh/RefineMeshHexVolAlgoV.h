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
#include <Core/Datatypes/Geometry.h>

// Base class for algorithm
#include <Core/Algorithms/Base/AlgorithmBase.h>

// for Windows support
#include <Core/Algorithms/Legacy/Fields/share.h>


namespace SCIRun{
		namespace Core{
				namespace Algorithms{
						namespace Fields{

class SCISHARE RefineMeshHexVolAlgoV : public AlgorithmBase
{
  public:  
    RefineMeshHexVolAlgoV();
      
		bool runImpl(FieldHandle input, FieldHandle& output, std::string select, double isoval) const; 
		//bool runImpl(FieldHandle input, FieldHandle& output) const; 

		virtual AlgorithmOutput run_generic(const AlgorithmInput& input) const override; 

protected:
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

    #ifdef HAVE_HASH_MAP
      struct edgepairhash
      {
        unsigned int operator()(const edgepair_t &a) const
        {
        #if defined(__ECC) || defined(_MSC_VER)
          hash_compare<unsigned int> h;
        #else
          hash<unsigned int> h;
        #endif
          return h((a.first<<3) ^ a.second);
        }
        
        #if defined(__ECC) || defined(_MSC_VER)
          // These are particularly needed by ICC's hash stuff
          static const size_t bucket_size = 4;
          static const size_t min_buckets = 8;
          
          // This is a less than function.
          bool operator()(const edgepair_t & a, const edgepair_t & b) const 
          {
            return edgepairless::less(a,b);
          }
        #endif // endif ifdef __ICC
      };

      #if defined(__ECC) || defined(_MSC_VER)
        typedef hash_map<edgepair_t,
                         VMesh::index_type,
                         edgepairhash> edge_hash_type;
      #else
        typedef hash_map<edgepair_t,
                         VMesh::index_type,
                         edgepairhash,
                         edgepairequal> edge_hash_type;
      #endif
    #else
      typedef std::map<edgepair_t,
                VMesh::Node::index_type,
                edgepairless> edge_hash_type;
    #endif
Point RIinterpolate(VMesh *refined,
                        VMesh::Node::array_type& onodes,
                        const Point &coordsp)  {
      Point result(0.0, 0.0, 0.0);
    
      double w[8];
      const double x = coordsp.x(), y = coordsp.y(), z = coordsp.z();
      w[0] = -((-1 + x)*(-1 + y)*(-1 + z));
      w[1] = x*(-1 + y)*(-1 + z);
      w[2] = -(x*y*(-1 + z));
      w[3] = (-1 + x)*y*(-1 + z);
      w[4] = (-1 + x)*(-1 + y)*z;
      w[5] = -(x*(-1 + y)*z);
      w[6] = x*y*z;
      w[7] = -((-1 + x)*y*z);

      Point p;
      for (int i = 0; i < 8; i++)
      {
        refined->get_point(p, onodes[i]);
        result += (p * w[i]).asVector();
      }
      
      return result;
    }
						
double RIinterpolateV(std::vector<double>& ivalues,
                        VMesh::Node::array_type& onodes,
                        const Point &coordsp){
      double w[8];
      const double x = coordsp.x(), y = coordsp.y(), z = coordsp.z();
      w[0] = -((-1 + x)*(-1 + y)*(-1 + z));
      w[1] = x*(-1 + y)*(-1 + z);
      w[2] = -(x*y*(-1 + z));
      w[3] = (-1 + x)*y*(-1 + z);
      w[4] = (-1 + x)*(-1 + y)*z;
      w[5] = -(x*(-1 + y)*z);
      w[6] = x*y*z;
      w[7] = -((-1 + x)*y*z);

      return (w[0]*ivalues[onodes[0]] + w[1]*ivalues[onodes[1]] +
        w[2]*ivalues[onodes[2]] + w[3]*ivalues[onodes[3]] +
        w[4]*ivalues[onodes[4]] + w[5]*ivalues[onodes[5]] +
        w[6]*ivalues[onodes[6]] + w[7]*ivalues[onodes[7]]);
    }
VMesh::Node::index_type add_point(VMesh *refined,
                                      VMesh::Node::array_type &nodes,
                                      const int *reorder, 
                                      unsigned int a, 
                                      unsigned int b,
                                      double factor,
                                      std::vector<double>& ivalues,
                                      int basis_order)
    {
      Point coordsp;
      
      unsigned int ra = reorder[a];
      unsigned int rb = reorder[b];
      coordsp = Interpolate(hcoords[ra], hcoords[rb], factor);
      
      const Point inbetween = RIinterpolate(refined, nodes, coordsp);
      if (basis_order == 1) ivalues.push_back(RIinterpolateV(ivalues,nodes,coordsp));
      return refined->add_point(inbetween);
    }

    VMesh::Node::index_type add_point_convex(VMesh *refined,
                                      VMesh::Node::array_type &nodes,
                                      const int *reorder, 
                                      VMesh::index_type a, 
                                      VMesh::index_type b,
                                      std::vector<double>& ivalues,
                                      int basis_order)
    {
      VMesh::index_type ra = reorder[a];
      VMesh::index_type rb = reorder[b];
      Point coordsp = Interpolate(hcoords[ra], hcoords[rb], 1.0/3.0);
      
      const Point inbetween = RIinterpolate(refined, nodes, coordsp);

      if (basis_order == 1) ivalues.push_back(RIinterpolateV(ivalues,nodes,coordsp));
      return refined->add_point(inbetween);
    }


    VMesh::Node::index_type add_point_convex(VMesh *refined,
                                      VMesh::Node::array_type &nodes,
                                      Point coordsp,
                                      std::vector<double>& ivalues,
                                      int basis_order)
    {
      const Point inbetween = RIinterpolate(refined, nodes, coordsp);
      if (basis_order == 1) ivalues.push_back(RIinterpolateV(ivalues,nodes,coordsp));
      return refined->add_point(inbetween);
    }


    VMesh::Node::index_type lookup(VMesh *refined,
                                   edge_hash_type &edgemap,
                                   VMesh::Node::array_type &nodes,
                                   const int *reorder, 
                                   VMesh::index_type a, 
                                   VMesh::index_type b,
                                   double factor,
                                   std::vector<double>& ivalues,
                                   int basis_order)
    {
      edgepair_t ep;
      ep.first = nodes[reorder[a]]; 
      ep.second = nodes[reorder[b]];
      const edge_hash_type::iterator loc = edgemap.find(ep);
      if (loc == edgemap.end())
      {
        const VMesh::Node::index_type newnode =
          add_point(refined, nodes, reorder, a, b, factor,ivalues,basis_order);
        edgemap[ep] = newnode;
        return newnode;
      }
      else
      {
        return (*loc).second;
      }
    }

    VMesh::Node::index_type lookup_convex(VMesh *refined,
                                   edge_hash_type &edgemap,
                                   VMesh::Node::array_type &onodes,
                                   const int *reorder, 
                                   VMesh::index_type a, 
                                   VMesh::index_type b,
                                   std::vector<double>& ivalues,
                                   int basis_order)
    {
      edgepair_t ep;
      ep.first = onodes[reorder[a]]; 
      ep.second = onodes[reorder[b]];
      const edge_hash_type::iterator loc = edgemap.find(ep);
      if (loc == edgemap.end())
      {
        const VMesh::Node::index_type newnode =
          add_point_convex(refined, onodes, reorder, a, b,ivalues,basis_order);
        edgemap[ep] = newnode;
        return newnode;
      }
      else
      {
        return (*loc).second;
      }
    }


    inline unsigned int iedge(unsigned int a, unsigned int b)
    {
      return (1<<(7-a)) | (1<<(7-b));
    }

    inline unsigned int iface(unsigned int a, unsigned int b,
                       unsigned int c, unsigned int d)
    {
      return iedge(a, b) | iedge(c, d);
    }

    inline unsigned int iface3(unsigned int a, unsigned int b, unsigned int c)
    {
      return (1<<(7-a)) | (1<<(7-b)) | (1<<(7-c));
    }

    inline void set_table(int i, int pattern, int reorder)
    {
      pattern_table[i][0] = pattern;
      pattern_table[i][1] = reorder;
    }

    void init_pattern_table()
    {
      for (int i = 0; i < 256; i++)
      {
        set_table(i, -1, 0);
      }

      set_table(0, 0, 0);

      // Add corners
      set_table(1, 1, 7);
      set_table(2, 1, 6);
      set_table(4, 1, 5);
      set_table(8, 1, 4);
      set_table(16, 1, 3);
      set_table(32, 1, 2);
      set_table(64, 1, 1);
      set_table(128, 1, 0);

      // Add edges
      set_table(iedge(0, 1), 2, 0);
      set_table(iedge(1, 2), 2, 1);
      set_table(iedge(2, 3), 2, 2);
      set_table(iedge(3, 0), 2, 3);
      set_table(iedge(4, 5), 2, 5);
      set_table(iedge(5, 6), 2, 6);
      set_table(iedge(6, 7), 2, 7);
      set_table(iedge(7, 4), 2, 4);
      set_table(iedge(0, 4), 2, 8);
      set_table(iedge(1, 5), 2, 9);
      set_table(iedge(2, 6), 2, 10);
      set_table(iedge(3, 7), 2, 11);

      set_table(iface(0, 1, 2, 3), 4, 0);
      set_table(iface(0, 1, 5, 4), 4, 12);
      set_table(iface(1, 2, 6, 5), 4, 9);
      set_table(iface(2, 3, 7, 6), 4, 13);
      set_table(iface(3, 0, 4, 7), 4, 8);
      set_table(iface(4, 5, 6, 7), 4, 7);

      set_table(iface3(0, 1, 2), -4, 0);
      set_table(iface3(0, 1, 3), -4, 0);
      set_table(iface3(0, 2, 3), -4, 0);
      set_table(iface3(1, 2, 3), -4, 0);
      set_table(iedge(0, 2), -4, 0);
      set_table(iedge(1, 3), -4, 0);

      set_table(iface3(0, 1, 5), -4, 12);
      set_table(iface3(0, 1, 4), -4, 12);
      set_table(iface3(0, 5, 4), -4, 12);
      set_table(iface3(1, 5, 4), -4, 12);
      set_table(iedge(0, 5), -4, 0);
      set_table(iedge(1, 4), -4, 0);

      set_table(iface3(1, 2, 6), -4, 9);
      set_table(iface3(1, 2, 5), -4, 9);
      set_table(iface3(1, 6, 5), -4, 9);
      set_table(iface3(2, 6, 5), -4, 9);
      set_table(iedge(1, 6), -4, 0);
      set_table(iedge(2, 5), -4, 0);

      set_table(iface3(2, 3, 7), -4, 13);
      set_table(iface3(2, 3, 6), -4, 13);
      set_table(iface3(2, 7, 6), -4, 13);
      set_table(iface3(3, 7, 6), -4, 13);
      set_table(iedge(2, 7), -4, 0);
      set_table(iedge(3, 6), -4, 0);

      set_table(iface3(3, 0, 4), -4, 8);
      set_table(iface3(3, 0, 7), -4, 8);
      set_table(iface3(3, 4, 7), -4, 8);
      set_table(iface3(0, 4, 7), -4, 8);
      set_table(iedge(3, 4), -4, 0);
      set_table(iedge(0, 7), -4, 0);

      set_table(iface3(4, 5, 6), -4, 7);
      set_table(iface3(4, 5, 7), -4, 7);
      set_table(iface3(4, 6, 7), -4, 7);
      set_table(iface3(5, 6, 7), -4, 7);
      set_table(iedge(4, 6), -4, 0);
      set_table(iedge(5, 7), -4, 0);
                       
      set_table(255, 8, 0);

      for (int i = 0; i < 8; i++)
      {
        hcoords[i] = Point(hcoords_double[i][0],
                           hcoords_double[i][1],
                           hcoords_double[i][2]);
      }
    }

    void dice(VMesh *refined, 
              edge_hash_type &emap,
              VMesh::Node::array_type nodes,
              VMesh::index_type index, 
              VMesh::mask_type mask,
              std::vector<double>& ivalues,
              std::vector<double>& evalues,
              double vv,
              int basis_order);
 private:
  
    Point hcoords[8];
    int pattern_table[256][2];
    
    static int hex_reorder_table[14][8];
    static double hcoords_double[8][3];
    
    VMesh::index_type maxnode;
	};
}}}}

#endif
