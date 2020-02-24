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


#include <Core/Algorithms/FiniteElements/Periodic/DefinePeriodicBoundaries.h>

#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>


namespace SCIRunAlgo {

using namespace SCIRun;

class LinkElement {
public:
  int     row;
  int     col;
};

inline bool operator==(const LinkElement& s1,const LinkElement& s2)
{
  if ((s1.row == s2.row)&&(s1.col == s2.col)) return (true);
  return (false);
}

inline bool operator<(const LinkElement& s1, const LinkElement& s2)
{
  if (s1.row < s2.row) return(true);
  if (s1.row == s2.row) if (s1.col < s2.col) return(true);
  return (false);
}

bool
DefinePeriodicBoundariesAlgo::run(FieldHandle& input,
                                  MatrixHandle& periodic_nodelink,
                                  MatrixHandle& periodic_delemlink)
{
  /// @todo: add progress reporting
  algo_start("DefinePeriodicBoundaries");

  // Get parameters
  bool linkx = get_bool("link_x_boundary");
  bool linky = get_bool("link_y_boundary");
  bool linkz = get_bool("link_z_boundary");
  double tol = get_scalar("tolerance");
  bool build_nodelink = get_bool("build_periodic_nodelink");
  bool build_delemlink = get_bool("build_periodic_delemlink");


  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  FieldInformation fi(input);

  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    algo_end(); return (false);
  }

  if (!(fi.is_volume()||fi.is_surface()||fi.is_curve()))
  {
    error("This function is only defined for curve, surface and volume meshes");
    algo_end(); return (false);
  }

  // Setup dynamic files

  VMesh* imesh = input->vmesh();
  if (imesh == 0)
  {
    error("No mesh associated with input field");
    algo_end(); return (false);
  }

#ifdef HAVE_HASH_MAP
  typedef hash_multimap<int,VMesh::DElem::index_type> delemidxmap_type;
  typedef std::vector<Point> pointmap_type;
#else
  typedef std::multimap<int, VMesh::DElem::index_type> delemidxmap_type;
  typedef std::vector<Point> pointmap_type;
#endif

  // Information to build:

  // A list of all the nodes that are at an delem
  std::vector<VMesh::Node::index_type> nodelist;

  // A list of all the delems that are at an delem
  VMesh::DElem::array_type delemlist;

  imesh->synchronize(Mesh::DELEMS_E|Mesh::ELEM_NEIGHBORS_E|Mesh::EPSILON_E);
  if (tol < 0.0) tol = imesh->get_epsilon();

  VMesh::Node::size_type numnodes;
  VMesh::DElem::size_type numdelems;
  imesh->size(numnodes);
  imesh->size(numdelems);

  pointmap_type pointmap(numnodes);
  delemidxmap_type delemidx;

  // Find all the delems that are at the delem
  // I.E. find the field boundary

  {
    VMesh::Elem::iterator be, ee;
    VMesh::Elem::index_type nci, ci;
    VMesh::DElem::array_type delems;
    VMesh::Node::array_type nodes;

    std::vector<VMesh::Node::index_type> nodelist_create;

    imesh->begin(be);
    imesh->end(ee);

    while (be != ee)
    {
      ci = *be;
      imesh->get_delems(delems,ci);
      for (size_t p =0; p < delems.size(); p++)
      {
        if(!(imesh->get_neighbor(nci,ci,delems[p])))
        {
          imesh->get_nodes(nodes,delems[p]);
          for (size_t q=0; q<nodes.size(); q++)
          {
            nodelist_create.push_back(nodes[q]);
          }
          delemlist.push_back(delems[p]);
        }
      }
      ++be;
    }

    std::sort(nodelist_create.begin(),nodelist_create.end());
    if (nodelist_create.size() > 0) nodelist.push_back(nodelist_create[0]);
    size_t v = 0;
    for (size_t w = 1; w < nodelist_create.size(); w++)
    {
      if (nodelist_create[w] != nodelist[v]) {nodelist.push_back(nodelist_create[w]); v++; }
    }
  }


  // We now have the boundary
  // Now determine the shifts in the mesh and points of origin

  // Build a nodemap of all the nodes we are interested in
  for (size_t r=0; r<nodelist.size(); r++)
  {
    VMesh::Node::index_type idx;
    Point p;

    imesh->get_center(p,nodelist[r]);
    pointmap[nodelist[r]] = p;
  }

  /// @todo: Need to replace this with searchgrid implementation

  double shiftx = 0.0;
  double shifty = 0.0;
  double shiftz = 0.0;

  double x0 = 0.0;
  double y0 = 0.0;
  double z0 = 0.0;

  double minx = 0.0;
  double miny = 0.0;
  double minz = 0.0;
  double maxx = 0.0;
  double maxy = 0.0;
  double maxz = 0.0;

  double xmul = 0.0;
  double ymul = 0.0;
  double zmul = 0.0;

  double tol2 = tol*tol;

  {
    Point p, xmin, ymin, zmin;

    if (nodelist.size() > 0)
    {
      p = pointmap[nodelist[0]];
      xmin = p;
      ymin = p;
      zmin = p;

      minx = p.x();
      maxx = p.x();
      miny = p.y();
      maxy = p.y();
      minz = p.z();
      maxz = p.z();
    }

    for (size_t r=0; r<nodelist.size();r++)
    {
      p = pointmap[nodelist[r]];
      if (p.x() < xmin.x()) xmin = p;
      if (p.y() < ymin.y()) ymin = p;
      if (p.z() < zmin.z()) zmin = p;

      if (p.x() < minx) minx = p.x();
      if (p.x() > maxx) maxx = p.x();
      if (p.y() < miny) miny = p.y();
      if (p.y() > maxy) maxy = p.y();
      if (p.z() < minz) minz = p.z();
      if (p.z() > maxz) maxz = p.z();
    }

    for (size_t r=0; r<nodelist.size();r++)
    {
      p = pointmap[nodelist[r]];
      if (((p.x()-xmin.x())> shiftx) && ((p.y()-xmin.y())*(p.y()-xmin.y()) + (p.z()-xmin.z())*(p.z()-xmin.z()) < tol2)) shiftx = p.x()-xmin.x();
      if (((p.y()-ymin.y())> shifty) && ((p.x()-ymin.x())*(p.x()-ymin.x()) + (p.z()-ymin.z())*(p.z()-ymin.z()) < tol2)) shifty = p.y()-ymin.y();
      if (((p.z()-zmin.z())> shiftz) && ((p.y()-zmin.y())*(p.y()-zmin.y()) + (p.x()-zmin.x())*(p.x()-zmin.x()) < tol2)) shiftz = p.z()-zmin.z();
    }

    if (linkx) { if ((shiftx) > tol ) xmul = 250/(shiftx); } else { if((maxx-minx) > 0.0) xmul = 250/(maxx-minx); linkx = false;}
    if (linky) { if ((shifty) > tol ) ymul = 250/(shifty); } else { if((maxy-miny) > 0.0) ymul = 250/(maxy-miny); linky = false;}
    if (linkz) { if ((shiftz) > tol ) zmul = 250/(shiftz); } else { if((maxz-minz) > 0.0) zmul = 250/(maxz-minz); linkz = false;}

    x0 = xmin.x();
    y0 = ymin.y();
    z0 = zmin.z();
  }


  {
    double center_x = 0.5*(minx+maxx);
    double center_y = 0.5*(miny+maxy);
    double center_z = 0.5*(minz+maxz);

    for (size_t r=0; r<nodelist.size();r++)
    {
      Point p, mp;
      imesh->get_center(p,nodelist[r]);

      mp = p;
      if (linkx) mp.x(fmod((p.x()-x0+center_x),shiftx)-center_x);
      if (linky) mp.y(fmod((p.y()-y0+center_y),shifty)-center_y);
      if (linkz) mp.z(fmod((p.z()-z0+center_z),shiftz)-center_z);

      pointmap[nodelist[r]] = mp;
    }
  }

  // Build a key map for each delem

  size_t delemcnt = 0;
  {
    VMesh::size_type listsize = static_cast<VMesh::size_type>(delemlist.size());
    for (VMesh::index_type r=0; r<listsize;r++)
    {
      VMesh::DElem::index_type idx;
      VMesh::Node::array_type nodes;
      int key;
      Point p;

      imesh->get_nodes(nodes,delemlist[r]);
      for (size_t q=0;q<nodes.size();q++)
      {
        p = pointmap[nodes[q]];

        key = static_cast<int>((p.x()-minx)*xmul);
        key += (static_cast<int>((p.y()-miny)*ymul))<<8;
        key += (static_cast<int>((p.z()-minz)*zmul))<<16;

        delemidx.insert(delemidxmap_type::value_type(key,delemlist[r]));
      }
      delemcnt = nodes.size();
    }
  }

  // Set up the translation table: which node it linked to which node
  std::vector<LinkElement> nodelink;
  std::vector<LinkElement> elemlink;


  // Main loop connect everything

  {
    VMesh::DElem::index_type idx, idx2;
    VMesh::Node::array_type nodes,nodes2;
    VMesh::DElem::array_type::iterator it, it_end;
    std::pair<delemidxmap_type::iterator,delemidxmap_type::iterator> lit;
    int key;

    it = delemlist.begin();
    it_end = delemlist.end();

    idx = (*it);
    imesh->get_nodes(nodes,idx);
    std::vector<Point> points(nodes.size());
    std::vector<unsigned int> delemlink(nodes.size());

    while (it != it_end)
    {
      idx = (*it);

      imesh->get_nodes(nodes,idx);
      for (size_t w=0; w<delemcnt; w++) imesh->get_center(points[w],nodes[w]);

      Point p;
      p = pointmap[nodes[0]];
      key = static_cast<int>((p.x()-minx)*xmul);
      key += (static_cast<int>((p.y()-miny)*ymul))<<8;
      key += (static_cast<int>((p.z()-minz)*zmul))<<16;

      bool founddelem = false;

      for (int x = -1; (x < 2)&&(founddelem == false); x++)
      {
        for (int y = -256; (y < 257)&&(founddelem == false); y += 256)
        {
          for (int z = -65536; (z < 65537)&&(founddelem == false); z += 65536)
          {
            lit = delemidx.equal_range(key+x+y+z);
            while (lit.first != lit.second)
            {
              bool foundit = true;
              idx2 = (*(lit.first)).second;

              if (idx == idx2) {  ++(lit.first); continue; }

              imesh->get_nodes(nodes2,idx2);
              for (unsigned int w=0;(w<delemcnt)&&(foundit == true); w++)
              {
                imesh->get_center(p,nodes2[w]);
                bool success = false;
                for (unsigned int v=0;v<delemcnt;v++)
                {
                  Vector vec(p - points[v]);
                  if (vec.length2() <= tol2) { success = true; break;}
                }
                if (success) { foundit = false; break;}
              }

              if (foundit)
              {
                for (unsigned int w=0;(w<delemcnt)&&(foundit == true); w++)
                {
                  Point p = pointmap[nodes[w]];
                  bool success = false;
                  for (unsigned int v=0;v<delemcnt;v++)
                  {
                    Vector vec(p-pointmap[nodes2[v]]);
                    if (vec.length2() <= tol2) { delemlink[w] = v; success = true; break;}
                  }
                  if (!success) { foundit = false; break; }
                }

                if (foundit)
                {
                  LinkElement elem;
                  founddelem = true;

                  for (unsigned int w=0;w<delemcnt; w++)
                  {
                    VMesh::index_type i1 = nodes[w];
                    VMesh::index_type i2 = nodes2[delemlink[w]];
                    elem.row = i1; elem.col = i2; nodelink.push_back(elem);
                    elem.row = i2; elem.col = i1; nodelink.push_back(elem);
                  }
                  elem.row =idx; elem.col = idx2; elemlink.push_back(elem);
                  elem.row =idx2; elem.col = idx; elemlink.push_back(elem);
                }
              }
              ++(lit.first);
              if (founddelem) break;
            }
          }
        }
      }
      ++it;
    }
  }

  if (build_nodelink)
  {
    std::sort(nodelink.begin(),nodelink.end());
    size_type nnz = 0;
    if (nodelink.size() >0)
    {
      index_type p = 0;
      nnz = 1;
      for (size_t q=0; q< nodelink.size();q++)
      {
        if (nodelink[q] == nodelink[p]) continue;
        p = q; nnz++;
      }
    }

    // reserve memory

    std::vector<index_type> rows(numnodes+1);
    std::vector<index_type> cols(nnz);

    index_type kk = 0;
    size_t p = 0;
    size_t q = 0;
    for (index_type r = 0; r < numnodes; r++)
    {
      rows[r] = kk;
      for (; q < nodelink.size();q++)
      {
        if (nodelink[q].row > r) { break; }
        if ((q==0)||(!(nodelink[p] == nodelink[q])))
        {
          p = q;
          cols[kk] = nodelink[q].col;
          kk++;
        }
      }
    }
    rows[numnodes] = kk;

    // 2nd correction, link indirect nodes

    std::vector<index_type> buffer(16);

    kk = 0;
    for (index_type r = 0; r< numnodes; r++)
    {
      index_type h = 0;
      index_type hh = 1;
      buffer[0] = r;

      while (h<hh)
      {
        index_type s = buffer[h];
        for (index_type u=rows[s]; u < rows[s+1]; u++)
        {
          index_type v;
          for (v=0; v < hh; v++) if (cols[u] == buffer[v]) break;
          if (v == hh) { buffer[hh] = cols[u]; hh++; }
        }
        h++;
      }
      kk+= hh;
    }

    nnz = kk;

    SparseRowMatrix::Data sparseData(numnodes+1, nnz);

    if (!sparseData.allocated())
    {
      error("Could not allocate memory for matrix");
      algo_end(); return (false);
    }
    const SparseRowMatrix::Rows& nrows = sparseData.rows();
    const SparseRowMatrix::Columns& ncols = sparseData.columns();
    const SparseRowMatrix::Storage& nvals = sparseData.data();

    kk = 0;
    nrows[0] = 0;
    for (index_type r = 0; r< numnodes; r++)
    {
      index_type h = 0;
      index_type hh = 1;
      buffer[0] = r;

      while (h<hh)
      {
        index_type s = buffer[h];
        for (index_type u=rows[s]; u < rows[s+1]; u++)
        {
          index_type v;
          for (v=0; v < hh; v++) if (cols[u] == buffer[v]) break;
          if (v == hh) { buffer[hh] = cols[u]; hh++;}
        }
        h++;
      }

      std::sort(buffer.begin(),buffer.begin()+hh);
      for (index_type s=0;s<hh;s++)
      {
        ncols[kk] = buffer[s];
        nvals[kk] = 1.0;
        kk++;
      }
      nrows[r+1] = kk;
    }

    rows.clear();
    cols.clear();

    periodic_nodelink = new SparseRowMatrix(numnodes,numnodes,sparseData,nnz);

    if (periodic_nodelink.get_rep() == 0)
    {
      error("Could not build PeriocicNodeLink mapping matrix");
      return (false);
    }
  }

  if (build_delemlink)
  {
    std::sort(elemlink.begin(),elemlink.end());
    size_type nnz = 0;
    if (elemlink.size() >0)
    {
      index_type p = 0;
      nnz = 1;
      for (size_t q=0; q< elemlink.size();q++)
      {
        if (elemlink[q] == elemlink[p]) continue;
        p = q; nnz++;
      }
    }

    SparseRowMatrix::Data sparseData(numdelems+1, nnz);

    if (!sparseData.allocated())
    {
      error("Could not allocate memory for matrix");
      algo_end(); return (false);
    }
    const SparseRowMatrix::Rows& rows = sparseData.rows();
    const SparseRowMatrix::Columns& cols = sparseData.columns();
    const SparseRowMatrix::Storage& vals = sparseData.data();

    index_type p = 0;
    index_type kk = 0;
    index_type q = 0;
    for (index_type r = 0; r < numdelems; r++)
    {
      rows[r] = kk;
      for (; q < static_cast<index_type>(elemlink.size());q++)
      {
        if (elemlink[q].row > r) { break; }
        if ((q==0)||(!(elemlink[p] == elemlink[q])))
        {
          p = q;
          cols[kk] = elemlink[q].col;
          vals[kk] = 1.0;
          kk++;
        }
      }
    }

    rows[numdelems] = kk;
    periodic_delemlink = new SparseRowMatrix(numdelems,numdelems,sparseData,nnz);

    if (periodic_delemlink.get_rep() == 0)
    {
      error("Could not build mapping matrix");
      algo_end(); return (false);
    }
  }

  algo_end(); return (true);
}


} // End namespace SCIRunAlgo
