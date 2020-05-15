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


///
///@file  InsertHexVolSheetAlongSurface.cc
///@brief Insert a layer of hexes.
///
///@author
///   Jason Shepherd
///   Department of Computer Science
///   University of Utah
///@date   March 2006
///

#include <Core/Util/StringUtil.h>

#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/BBox.h>

#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>

#include <vector>
#include <set>

#include <string>
#include <iostream>

namespace SCIRun {


class Model
{
  public:
    Model() :
      is_bounding_box_valid_(false),
      is_centroid_valid_(false) {}

    virtual ~Model() {}

    virtual BBox& bounding_box();
    virtual Point& centroid();

  protected:
    virtual void compute_bounding_box() = 0;
    virtual void compute_centroid() = 0;

    void invalidate_all();

    BBox  bounding_box_;
    Point centroid_;
    bool  is_bounding_box_valid_;
    bool  is_centroid_valid_;
};



BBox&
Model::bounding_box()
{
	if (!is_bounding_box_valid_)
  {
		compute_bounding_box();
		is_bounding_box_valid_ = true;
	}
	return (bounding_box_);
}

Point&
Model::centroid()
{
	if (!is_centroid_valid_)
  {
		compute_centroid();
		is_centroid_valid_ = true;
	}
	return centroid_;
}

void
Model::invalidate_all()
{
  is_bounding_box_valid_ = false;
  is_centroid_valid_ = false;
}

// an hexahedral cell stored in the following way:
//
//     /+-------------|
//    /	|6     	     /|7
//   /	|      	   -/ |
//  / 	|      	  /   |
// /-------------/    |
//2|   	|      	 |3   |
// |  	|     	 |    |
// |  	|     	 |    |
// |  	|     	 |    |
// |  	|        |    |
// |   	|--------|-----
// |  /- 4     	 |  -/ 5
// |/-         	 |-/
// /-------------/
//0             1
//
//NOTE: The following definition is incorrect...  The neighbors array is simply a random accounting of which hexes are adjacent through faces to other hexes.  Ordering is not important in this algorithm...
// Neighbors: 0: through face 0132
//            1: through face 5467
//            2: through face 0264
//            3: through face 1573
//            4: through face 2376
//            5: through face 4510

class Hex
{
  public:
    int verts[8];
};

class HexMesh: public Model
{
  public:
    HexMesh() {};

    std::vector<Point> points;
    std::vector<Hex> hexes;

    virtual void compute_bounding_box();
    virtual void compute_centroid();
};



void HexMesh::compute_bounding_box()
{
  bounding_box_ = BBox(points);
}

void HexMesh::compute_centroid()
{
  centroid_ = SCIRun::centroid(points);
}


class TriangleMeshVertex {
  public:
    TriangleMeshVertex(Point p) : point(p), someface(-1) {}

    Point point;
    Vector normal;
    int someface;
};

class TriangleMeshFace {
public:
	TriangleMeshFace(const int v[3])
  {
		verts[0] = v[0];
		verts[1] = v[1];
		verts[2] = v[2];
		nbrs[0]  = nbrs[1] = nbrs[2] = -1;
	}

	int VertIndex(int v) const
  {
		if (verts[0] == v)	return 0;
		if (verts[1] == v)	return 1;
		if (verts[2] == v)	return 2;
		return -1;
	}

	int EdgeIndexCCW(int v0, int v1) const
  {
		int vi = VertIndex(v0);
		if (vi<0) return -1;

		if (verts[(vi+1)%3] == v1) return vi;

		return -1;
	}

	int verts[3];	// ccw order
	int nbrs[3];	// nbrs[0] shares verts 0,1, 1 shares 1,2, 2 shares 2,0
    // -1 - boundary
};

class TriangleMesh : public Model  {

  public:
    typedef std::vector<TriangleMeshVertex> vertex_list;
    typedef std::vector<TriangleMeshFace> face_list;

    TriangleMesh();
    ~TriangleMesh();

    void add_point( double x, double y, double z );
    void add_tri( int* node_id_array );

    // clear the whole mesh
    void Clear();

    // these are dangerous if the mesh has been changed!
    int FaceIndex(const TriangleMeshFace &f) const
    {
      return (int)(&f - &faces[0]);
    }

    int VertexIndex(const TriangleMeshVertex &v) const
    {
      return (int)(&v - &verts[0]);
    }

    Vector FaceNormal(const TriangleMeshFace &f) const
    {
      Vector e1 = verts[f.verts[1]].point - verts[f.verts[0]].point;
      Vector e2 = verts[f.verts[2]].point - verts[f.verts[0]].point;
      Vector norm = Cross(e1,e2);
      norm.normalize();
      return norm;
    }

    void SetNormals();

    void compute_bounding_box();
    void compute_centroid();


    class VertexFaceIterator {
      public:
        VertexFaceIterator(TriangleMesh& _mesh, int vi);
        bool done();
        VertexFaceIterator& operator++();
        TriangleMeshFace& operator*();

      private:
        bool first;
        int cur_face;
        int first_face;
        int vertex;
        TriangleMesh *mesh;
     };

    void build_structures(const std::vector<int> &facemap,
                          const std::vector<int> &vertmap);
    void IdentityMap(std::vector<int> & map, int size);

    vertex_list	verts;
    face_list	faces;
};

//begin inline definitions....
inline
TriangleMesh::TriangleMesh()
{
	Model::invalidate_all();
}

inline
TriangleMesh::~TriangleMesh()
{
}

// Model stuff
inline void
TriangleMesh::compute_bounding_box()
{
  Model::bounding_box_ = BBox(verts[0].point, verts[0].point);
	for (vertex_list::const_iterator v=verts.begin(); v!=verts.end(); ++v)
  {
    Model::bounding_box_.extend(v->point);
	}
	Model::is_bounding_box_valid_ = true;
}

inline void
TriangleMesh::compute_centroid()
{
	Model::centroid_ = Point(0.0,0.0,0.0);

	for (vertex_list::const_iterator v=verts.begin(); v!=verts.end(); ++v)
  {
		Model::centroid_ += v->point;
	}
	Model::centroid_ *= (1.0 / verts.size());
	Model::is_centroid_valid_ = true;
}

inline void
TriangleMesh::add_point( double x, double y, double z )
{
  verts.push_back(TriangleMeshVertex(Point(x, y, z)));
}

inline void
TriangleMesh::add_tri( int* node_id_array )
{
  faces.push_back(TriangleMeshFace(node_id_array));
}

inline void
TriangleMesh::build_structures(const std::vector<int>& /*facemap*/, const std::vector<int> &vertmap)
{
    // convert all the indices to the array indices
	for (face_list::iterator f=faces.begin(); f!=faces.end(); ++f)
  {
		for (int i=0; i<3; i++)
    {
			f->verts[i] = vertmap[f->verts[i]];
		}
	}

  // set the somefaces
	for (vertex_list::iterator v=verts.begin(); v!=verts.end(); ++v)
  {
		v->someface = -1;
	}
	for (face_list::iterator f=faces.begin(); f!=faces.end(); ++f)
  {
		for (int i=0; i<3; i++)
    {
			verts[f->verts[i]].someface = FaceIndex(*f);
		}
	}

  // build the adjacency info

	std::vector< std::vector<int> > vertfaces(verts.size());
	for (size_t i=0; i<vertfaces.size(); i++)
  {
		vertfaces[i].reserve(7);
	}

	for (face_list::iterator f=faces.begin(); f!=faces.end(); ++f)
  {
		for (int i=0; i<3; i++)
    {
			vertfaces[f->verts[i]].push_back(FaceIndex(*f));
		}
	}

	for (face_list::iterator f=faces.begin(); f!=faces.end(); ++f)
  {
		for (int i=0; i<3; i++)
    {

			int v0 = f->verts[i];
			int v1 = f->verts[(i+1)%3];

        // look for a face with the edge v1,v0
			bool found=false;
			for (size_t vfi=0; vfi<vertfaces[v0].size(); vfi++)
      {
				int vf = vertfaces[v0][vfi];
				if (faces[vf].EdgeIndexCCW(v1, v0) != -1)
        {
					f->nbrs[i] = vf;
					if (found)
          {
						std::cerr<<"more than one matching triangle found: faces["<<vf<<"]"<<std::endl;
					}
					found=true;
				}
			}
		}
	}

	SetNormals();

	Model::invalidate_all();
}

// set the normals
inline void
TriangleMesh::SetNormals()
{
	for (vertex_list::iterator v=verts.begin(); v!=verts.end(); ++v)
  {
		v->normal = Vector(0.0,0.0,0.0);
		for (VertexFaceIterator f(*this, VertexIndex(*v)); !f.done(); ++f)
    {
			Vector e1 = verts[(*f).verts[1]].point - verts[(*f).verts[0]].point;
			Vector e2 = verts[(*f).verts[2]].point - verts[(*f).verts[0]].point;
			Vector fn = Cross(e1,e2);
			if (fn.length() == 0)
      {
				std::cerr<<"skipping normal of degenerate face "<<FaceIndex(*f)<<std::endl;
			}
      else
      {
				fn.normalize();
				v->normal += FaceNormal(*f);
			}
		}
		v->normal.normalize();
	}
}

// iterators
inline
TriangleMesh::VertexFaceIterator::VertexFaceIterator(TriangleMesh& _mesh, int vi)
{
	first=true;
	mesh = &_mesh;
	vertex = vi;

  // rotate clockwise as far as possible
	cur_face = mesh->verts[vi].someface;
	if (cur_face != -1)
  {
		do
    {
			TriangleMeshFace &f = mesh->faces[cur_face];
			int cindex = f.VertIndex(vi);
			if (f.nbrs[cindex] == -1) break;
			cur_face = f.nbrs[cindex];
		}
    while (cur_face != mesh->verts[vi].someface);
		first_face = cur_face;
	}
}

inline bool
TriangleMesh::VertexFaceIterator::done()
{
	if (cur_face == -1) return true;
	return false;
}

TriangleMeshFace&
TriangleMesh::VertexFaceIterator::operator*()
{
	return mesh->faces[cur_face];
}

inline TriangleMesh::VertexFaceIterator&
TriangleMesh::VertexFaceIterator::operator++()
{

	TriangleMeshFace &f = mesh->faces[cur_face];
	int nindex = (f.VertIndex(vertex) + 2) % 3;
	cur_face = f.nbrs[nindex];

	if (cur_face==first_face)
      cur_face = -1;

	first = false;

	return *this;
}

inline void
TriangleMesh::Clear()
{
	verts.clear();
	faces.clear();
}

inline void
TriangleMesh::IdentityMap(std::vector<int> &map, int size)
{
	map.resize(size);
	for (int i=0; i<size; i++)
  {
		map[i] = i;
	}
}


// kd tree that allows overlapping bounding boxes - each object only exists in the tree in one place

// DISTBOXCLASS	must provide two typenames and two functions:
// typename DISTBOXCLASS::Box3
// typename DISTBOXCLASS::Point3
// DISTBOXCLASS::bounding_box(const OBJECT &) const;
// DISTBOXCLASS::distance(const OBJECT &, const typename DISTBOXCLASS::Point3 &) const;

//TO DO: SHould see whether to move this to containers as it may be generally more useful

template <typename OBJECT, typename DISTBOXCLASS>
class BoxKDTree {

public:

  enum { BOXKDTREE_MAXLEAFSIZE = 10 };

	BoxKDTree()
  {
		children[0] = children[1] = NULL;
	}
	~BoxKDTree()
  {
		if (children[0]) delete children[0];
		if (children[1]) delete children[1];
	}

	BoxKDTree(const std::vector<OBJECT> &iobjects, const DISTBOXCLASS &boxclass, int axis=0) {

		children[0] = children[1] = NULL;

		ReBuild(iobjects, boxclass, axis);
	}

	void ReBuild(const std::vector<OBJECT> &iobjects, const DISTBOXCLASS &boxclass, int axis=0) {

		if (children[0]) delete children[0];
		if (children[1]) delete children[1];
		children[0] = children[1] = NULL;

		objects = iobjects;

      // make the bounding box
    bbox = boxclass.bounding_box(objects[0]);
		for (unsigned i=1; i<objects.size(); i++) {
			bbox.extend(boxclass.bounding_box(objects[i]));
		}

		Split(boxclass, axis);
	}

    // get all the objects who's bounding boxes intersect the given box
	void GetIntersectedBoxes(const DISTBOXCLASS &boxclass, const BBox &ibox, std::vector<OBJECT> &intersected) const
  {
      // check our bounding box
		if (ibox.intersect(bbox) == BBox::OUTSIDE)
    {
			return;
		}

      // check any leaf objects
		for (size_t i=0; i<objects.size(); i++)
    {
			BBox obox = boxclass.bounding_box(objects[i]);

			if (ibox.intersect(obox) != BBox::OUTSIDE)
      {
				intersected.push_back(objects[i]);
			}
		}

      // try going into the children
		if (children[0])	children[0]->GetIntersectedBoxes(boxclass, ibox, intersected);
		if (children[1])	children[1]->GetIntersectedBoxes(boxclass, ibox, intersected);
	}

	void Insert(const DISTBOXCLASS &boxclass, const OBJECT &o, int axis=0)
  {
		BBox obox = boxclass.bounding_box(o);

      // figure out which side we want to put it in
		int addside = -1;

		if (children[0] && children[1])
    {
      // see which insertion would result in a smaller bounding box overlap

			BBox c0e = BBox(children[0]->bbox, obox);
			BBox c1e = BBox(children[1]->bbox, obox);

			bool intersect0 = c0e.intersect(children[1]->bbox) != BBox::OUTSIDE;
			bool intersect1 = c1e.intersect(children[0]->bbox) != BBox::OUTSIDE;

			if (intersect0 && !intersect1)
      {
				addside = 1;
			}
      else if (!intersect0 && intersect1)
      {
				addside = 0;
			}
      else if (intersect0 && intersect1)
      {
          // figure out which way causes the smallest overlap

        Point t1 = Max(c0e.min(),children[1]->bbox.min());
        Point t2 = Min(c0e.max(),children[1]->bbox.max());
        BBox ibox0(t1, t2);

        Point t3 = Max(c1e.min(),children[0]->bbox.min());
        Point t4 = Min(c1e.max(),children[0]->bbox.max());
        BBox ibox1(t3, t4);

				if (ibox0.x_length()*ibox0.y_length()*ibox0.z_length() < ibox1.x_length()*ibox1.y_length()*ibox1.z_length())
            addside = 0;
				else
            addside = 1;
			}
      else
      {
          // adding to neither would cause an intersection - add to the one that increases volume the least
				if (c0e.x_length()*c0e.y_length()*c0e.z_length() < c1e.x_length()*c1e.y_length()*c1e.z_length())
            addside = 0;
				else
            addside = 1;
			}
		}
    else if (children[0] && !children[1])
    {
			addside = 0;
		}
    else if (!children[0] && children[1])
    {
			addside = 1;
		}

      // expand our own bounding box
		bbox = (addside==-1 && objects.size()==0) ? obox : BBox(bbox, obox);

		if (addside == -1)
    {
			objects.push_back(o);
			Split(boxclass, axis);
		}
    else
    {
			children[addside]->Insert(boxclass, o, (axis+1)%3);
		}
	}

	bool Remove(const DISTBOXCLASS &boxclass, const OBJECT &o)
  {
		if (bbox.intersect(boxclass.bounding_box(o)) == BBox::OUTSIDE)
        return false;

      // first check in the list of objects at this node
		for (size_t i=0; i<objects.size(); i++)
    {
			if (o == objects[i])
      {

          // remove the object from the list
				if (i != objects.size()-1)
        {
					objects[i] = objects.back();
				}
				objects.pop_back();

          // recompute the bounding box
				if (objects.size() > 0)
        {
          bbox = boxclass.bounding_box(objects[0]);
					for (unsigned i=1; i<objects.size(); i++)
          {
						bbox = BBox(bbox, boxclass.bounding_box(objects[i]));
					}
				}
        else
        {
          bbox = BBox(Point(-1.0,-1.0,-1.0),Point(1.0,1.0,1.0));
   			}

				return true;
			}
		}

      // if we got here, we didn't find a match is the object list - check the children
		for (int c=0; c<2; c++)
    {
			if (children[c] && children[c]->Remove(boxclass, o))
      {
				int dangle = children[c]->dangling();
				if (dangle != -1)
        {
          // the child we removed from now has no leaf objects and a single child - prune it from the tree
					BoxKDTree *child = children[c];
					children[c] = child->children[dangle];
					child->children[dangle] = 0;
					delete child;
				}
        else if (children[c]->empty())
        {
            // the child is now completely empty
					delete children[c];
					children[c] = 0;
				}
				return true;
			}
		}

    // didn't find it anywhere!
		return false;
	}

private:
	void Split(const DISTBOXCLASS &boxclass, int axis=-1)
  {
      // check if we should stop splitting
		if (objects.size() >= BOXKDTREE_MAXLEAFSIZE)
    {
      // if we're not told what axis to use, use the biggest
			if (axis == -1)
      {
				double xl, yl, zl;
				xl = bbox.x_length();
				yl = bbox.y_length();
				zl = bbox.z_length();

				if (xl>yl && xl>zl) axis = 0;
				else if (yl>xl && yl>zl) axis = 1;
				else axis = 2;
			}

        // split the list by the axis
			std::vector<OBJECT> cobjects[2];

			if (objects.size() < 500)
      {
				std::vector< std::pair<double,OBJECT> > sorter(objects.size());

				size_t i;
				for (i=0; i<objects.size(); i++)
        {
					BBox obox = boxclass.bounding_box(objects[i]);
          Point c = obox.center();
					sorter[i] = std::pair<double,OBJECT>((c[axis]), objects[i]);
				}

				std::sort(sorter.begin(), sorter.end());

				for (i=0; i<sorter.size()/2; i++)
        {
					cobjects[0].push_back(sorter[i].second);
				}
				for ( ; i<sorter.size(); i++)
        {
					cobjects[1].push_back(sorter[i].second);
				}

			}
      else
      {
				for (size_t i=0; i<objects.size(); i++)
        {
					BBox obox = boxclass.bounding_box(objects[i]);

					if (obox.center()[axis] < bbox.center()[axis])
          {
						cobjects[0].push_back(objects[i]);
					}
          else
          {
						cobjects[1].push_back(objects[i]);
					}
				}
			}

			if ((cobjects[0].size() != 0) && (cobjects[1].size() != 0))
      {
        // actually have to split
				objects.clear();
				ASSERT(!children[0] && !children[1]);

				children[0] = new BoxKDTree(cobjects[0], boxclass, (axis+1)%3);
				children[1] = new BoxKDTree(cobjects[1], boxclass, (axis+1)%3);
			}
		}
	}

	bool empty()
  {
		return (!children[0] && !children[1] && !objects.size());
	}

	int dangling()
  {
		if (!objects.size())
    {
			if (children[0] && !children[1])	return 0;
			if (!children[0] && children[1])	return 1;
		}
		return -1;
	}

  // leaf node
	std::vector<OBJECT> objects;

  // internal node
	BoxKDTree* children[2];

	BBox bbox;
};



// edges used to determine face normals
const int hex_normal_edges[6][2] = {
  {0, 5}, {2, 6}, {10, 6}, {9, 7}, {1, 11}, {2, 9}};

///@class InsertHexVolSheetAlongSurfaceAlgo
/// @brief This module inserts a layer of hexes corresponding to the input TriSurfMesh.

class InsertHexVolSheetAlongSurfaceAlgo
{
  public:
    /// virtual interface.
    bool execute(
      ProgressReporter *reporter,
      FieldHandle hexfieldh, FieldHandle trifieldh,
      FieldHandle& side1field, FieldHandle& side2field,
      bool add_to_side1, bool add_layer );

    void load_hex_mesh( VMesh* hexfield );
    void load_tri_mesh( VMesh* trifield );

    void compute_intersections(
        ProgressReporter* mod,
        VMesh* original_mesh, VMesh *tri_mesh,
        VMesh* side1_mesh, VMesh* side2_mesh,
        bool add_to_side1, bool add_layer );

    bool interferes(const std::vector<Vector> &p, const Vector &axis, int split);

    bool intersects( const HexMesh &hexmesh, int hex_index,
                     const TriangleMesh &trimesh, int face_index);

    void compute_intersections_KDTree( ProgressReporter* pr,
                                       std::vector<int> &crosses,
                                       const TriangleMesh& trimesh,
                                       const HexMesh& hexmesh);

    static bool pair_less(const std::pair<double, unsigned int> &a,
                          const std::pair<double, unsigned int> &b)
    {
      return a.first < b.first;
    }

    class TriangleMeshFaceTree
    {
      public:

        TriangleMeshFaceTree(const TriangleMesh &m) : mesh(m) { }
        BBox bounding_box(int i) const
        {
          return BBox(mesh.verts[mesh.faces[i].verts[0]].point,
                      mesh.verts[mesh.faces[i].verts[1]].point,
                      mesh.verts[mesh.faces[i].verts[2]].point);
      }

      const TriangleMesh &mesh;
    };

  private:
    TriangleMesh trimesh;
    HexMesh      hexmesh;
};




bool
InsertHexVolSheetAlongSurfaceAlgo::execute(
    ProgressReporter *pr,
    FieldHandle hexfieldh,
    FieldHandle trifieldh,
    FieldHandle& side1field,
    FieldHandle& side2field,
    bool add_to_side1,
    bool add_layer )
{
  VMesh* mesh = hexfieldh->vmesh();
  VMesh* tri_mesh = trifieldh->vmesh();

  load_tri_mesh( tri_mesh );
  pr->update_progress( 0.05 );

  load_hex_mesh( mesh );
  pr->update_progress( 0.15 );


  FieldInformation fi(hexfieldh);
  /// @todo: Since the algorithm does not copy data
  // make the field no data. We should add some interpolation at some point
  fi.make_nodata();
  fi.make_hexvolmesh();

  side1field = CreateField(fi);
  side2field = CreateField(fi);
  side1field->copy_properties( hexfieldh.get_rep() );
  side2field->copy_properties( hexfieldh.get_rep() );

  VMesh* side1_mesh = side1field->vmesh();
  side1_mesh->copy_properties( mesh );

  VMesh* side2_mesh = side2field->vmesh();
  side2_mesh->copy_properties( mesh );

  compute_intersections( pr, mesh, tri_mesh, side1_mesh, side2_mesh, add_to_side1, add_layer );

  return (true);
}

void
InsertHexVolSheetAlongSurfaceAlgo::load_tri_mesh( VMesh* sci_trimesh )
{
  VMesh::Node::size_type num_nodes = sci_trimesh->num_nodes();
  VMesh::Elem::size_type num_elems = sci_trimesh->num_elems();

  for (VMesh::Node::index_type idx=0; idx<num_nodes;idx++)
  {
    Point p;
    sci_trimesh->get_center( p, idx );
    trimesh.add_point( p.x(), p.y(), p.z() );
  }

  VMesh::Node::array_type onodes;
  for (VMesh::Elem::index_type idx=0; idx<num_elems;idx++)
  {
    sci_trimesh->get_nodes(onodes, idx);
    int vi[3];
    vi[0] = static_cast<int>(onodes[0]);
    vi[1] = static_cast<int>(onodes[1]);
    vi[2] = static_cast<int>(onodes[2]);
    trimesh.add_tri( vi );
  }

  // We've read all the data - build the actual structures now.
  std::vector<int> facemap, vertmap;
  trimesh.IdentityMap(facemap, num_elems);
  trimesh.IdentityMap(vertmap, num_nodes);
  trimesh.build_structures(facemap, vertmap);
}

void
InsertHexVolSheetAlongSurfaceAlgo::load_hex_mesh( VMesh* sci_hexmesh )
{
  VMesh::Node::size_type num_nodes = sci_hexmesh->num_nodes();
  VMesh::Elem::size_type num_elems = sci_hexmesh->num_elems();

  hexmesh.hexes.resize( num_elems );
  hexmesh.points.resize( num_nodes );

  for(VMesh::Node::index_type idx=0;idx<num_nodes;idx++)
  {
    Point p;
    sci_hexmesh->get_center( p, idx );
    hexmesh.points[idx] = Point( p.x(), p.y(), p.z() );
  }

  sci_hexmesh->synchronize( Mesh::FACES_E );

  VMesh::Node::array_type onodes;
  for(VMesh::Elem::index_type idx=0;idx<num_elems;idx++)
  {
    sci_hexmesh->get_nodes(onodes, idx);
    hexmesh.hexes[idx].verts[0] = onodes[0];
    hexmesh.hexes[idx].verts[1] = onodes[1];
    hexmesh.hexes[idx].verts[2] = onodes[3];
    hexmesh.hexes[idx].verts[3] = onodes[2];
    hexmesh.hexes[idx].verts[4] = onodes[4];
    hexmesh.hexes[idx].verts[5] = onodes[5];
    hexmesh.hexes[idx].verts[6] = onodes[7];
    hexmesh.hexes[idx].verts[7] = onodes[6];
  }
}

/// Brief projects points on the axis, tests overlap.
bool
InsertHexVolSheetAlongSurfaceAlgo::interferes(const std::vector<Vector> &p,
                                                 const Vector &axis, int split )
{
  std::vector<Vector> v(p.size());
  std::vector<float> d(p.size());
  for (size_t i=0; i<p.size(); ++i)
  {
    // Project each point on axis by projected(d) = v.v^T.e
    v[i] = axis * Dot(axis,p[i]);
    // Get the signed distance to points from null space of v
    d[i] = static_cast<float>(Dot(v[i],axis));
  }

  const float mnh = *min_element(d.begin(), d.begin()+split);
  const float mxh = *max_element(d.begin(), d.begin()+split);
  const float mnt = *min_element(d.begin()+split, d.end());
  const float mxt = *max_element(d.begin()+split, d.end());

  const bool mntmxh = mnt <= mxh;
  const bool mxhmxt = mxh <= mxt;
  const bool mnhmnt = mnh <= mnt;
  const bool mnhmxt = mnh <= mxt;
  const bool mxtmxh = mxt <= mxh;
  const bool mntmnh = mnt <= mnh;

  return ((mntmxh && mxhmxt) || (mnhmnt && mntmxh) ||
      (mnhmxt && mxtmxh) || (mntmnh && mnhmxt));
}

bool
InsertHexVolSheetAlongSurfaceAlgo::intersects(const HexMesh &hexmesh,
                                         int hex_index,
                                         const TriangleMesh &trimesh,
                                         int face_index)
{
  const TriangleMeshFace &face = trimesh.faces[face_index];
  const Hex &hex = hexmesh.hexes[hex_index];

  Vector triangle_edges[3] = {
      trimesh.verts[face.verts[1]].point-trimesh.verts[face.verts[0]].point,
      trimesh.verts[face.verts[2]].point-trimesh.verts[face.verts[1]].point,
      trimesh.verts[face.verts[0]].point-trimesh.verts[face.verts[2]].point
  };

  Vector hex_edges[12] = {
      hexmesh.points[hex.verts[0]] - hexmesh.points[hex.verts[1]],
      hexmesh.points[hex.verts[2]] - hexmesh.points[hex.verts[3]],
      hexmesh.points[hex.verts[4]] - hexmesh.points[hex.verts[5]],
      hexmesh.points[hex.verts[6]] - hexmesh.points[hex.verts[7]],
      hexmesh.points[hex.verts[0]] - hexmesh.points[hex.verts[2]],
      hexmesh.points[hex.verts[1]] - hexmesh.points[hex.verts[3]],
      hexmesh.points[hex.verts[4]] - hexmesh.points[hex.verts[6]],
      hexmesh.points[hex.verts[5]] - hexmesh.points[hex.verts[7]],
      hexmesh.points[hex.verts[0]] - hexmesh.points[hex.verts[4]],
      hexmesh.points[hex.verts[1]] - hexmesh.points[hex.verts[5]],
      hexmesh.points[hex.verts[2]] - hexmesh.points[hex.verts[6]],
      hexmesh.points[hex.verts[3]] - hexmesh.points[hex.verts[7]]
  };

  std::vector<Vector> ps(11);
  for (int i=0; i<8; ++i)
      ps[i] = hexmesh.points[hex.verts[i]] - Point(0,0,0);
  for (int i=8; i<11; ++i)
      ps[i] = trimesh.verts[face.verts[i-8]].point - Point(0,0,0);
  for (int i=0; i<3; ++i)
      triangle_edges[i].normalize();

  for (int i=0; i<12; ++i)
  {
    hex_edges[i].normalize();
    for (int j=0; j<3; ++j)
        if (!interferes(ps, Cross(hex_edges[i],triangle_edges[j]), 8))
            return false;
  }

  for (int i=0; i<6; ++i)
    if (!interferes(ps, Cross(hex_edges[hex_normal_edges[i][0]],
                              hex_edges[hex_normal_edges[i][1]]), 8))
      return false;
  return interferes(ps, Cross(triangle_edges[1],triangle_edges[0]), 8);
}


void
InsertHexVolSheetAlongSurfaceAlgo::compute_intersections_KDTree(
    ProgressReporter *pr, std::vector<int> &crosses,
    const TriangleMesh& trimesh, const HexMesh& hexmesh )
{
  std::vector<int> kdfi;
  for (size_t i=0; i<trimesh.faces.size(); i++)
  {
    kdfi.push_back(i);
  }

  TriangleMeshFaceTree kdtreebbox(trimesh);

  BoxKDTree<int, TriangleMeshFaceTree> kdtree(kdfi, kdtreebbox);

  int total_hexes = (int)hexmesh.hexes.size();

  for (size_t h=0; h<hexmesh.hexes.size(); h++)
  {
    BBox hbbox;
    hbbox.extend(hexmesh.points[hexmesh.hexes[h].verts[0]]);
    hbbox.extend(hexmesh.points[hexmesh.hexes[h].verts[1]]);
    hbbox.extend(hexmesh.points[hexmesh.hexes[h].verts[2]]);
    hbbox.extend(hexmesh.points[hexmesh.hexes[h].verts[3]]);
    hbbox.extend(hexmesh.points[hexmesh.hexes[h].verts[4]]);
    hbbox.extend(hexmesh.points[hexmesh.hexes[h].verts[5]]);
    hbbox.extend(hexmesh.points[hexmesh.hexes[h].verts[6]]);
    hbbox.extend(hexmesh.points[hexmesh.hexes[h].verts[7]]);

    std::vector<int> possible;
    kdtree.GetIntersectedBoxes(kdtreebbox, hbbox, possible);

		for (int i=0; i<(int)possible.size(); i++)
    {
			if (intersects(hexmesh, h, trimesh, possible[i]))
      {
        crosses[h] = 0;
        break;
      }
    }

    if( h%100 == 0 )
    {
      double temp = 0.15 + 0.5*h/total_hexes;
      pr->update_progress( temp );
    }
  }
}


void
InsertHexVolSheetAlongSurfaceAlgo::compute_intersections(
    ProgressReporter* pr,
    VMesh* original_mesh,
    VMesh* tri_mesh,
    VMesh* side1_mesh,
    VMesh* side2_mesh,
    bool add_to_side1,
    bool add_layer )
{
#ifdef HAVE_HASH_MAP
# if defined(__ECC) || defined(_MSC_VER)
  typedef hash_map<VMesh::index_type,
    VMesh::index_type,
    hash_compare<unsigned int, std::equal_to<unsigned int> > > hash_type;
# else
  typedef hash_map<VMesh::index_type,
    VMesh::index_type,
    hash<unsigned int>,
    std::equal_to<unsigned int> > hash_type;
# endif
#else
  typedef std::map<VMesh::index_type,
    VMesh::index_type,
    std::less<unsigned int> > hash_type;
#endif

#ifdef HAVE_HASH_MAP
# if defined(__ECC) || defined(_MSC_VER)
  typedef hash_map<VMesh::index_type,
    VMesh::index_type,
    hash_compare<unsigned int, std::equal_to<unsigned int> > > hash_type2;
# else
  typedef hash_map<VMesh::index_type,
    VMesh::index_type,
    hash<unsigned int>,
    std::equal_to<unsigned int> > hash_type2;
# endif
#else
  typedef std::map<VMesh::index_type,
    VMesh::index_type,
    std::less<unsigned int> > hash_type2;
#endif

#ifdef HAVE_HASH_MAP
# if defined(__ECC) || defined(_MSC_VER)
  typedef hash_map<VMesh::index_type,
    VMesh::index_type,
    hash_compare<unsigned int, std::equal_to<unsigned int> > > hash_type3;
# else
  typedef hash_map<VMesh::index_type,
    VMesh::index_type,
    hash<unsigned int>,
    std::equal_to<unsigned int> > hash_type3;
# endif
#else
  typedef std::map<VMesh::index_type,
    VMesh::index_type,
    std::less<unsigned int> > hash_type3;
#endif

  std::vector<int> crosses(hexmesh.hexes.size(), -1);
  std::vector<int> hexes(hexmesh.hexes.size());

  for (int i=0; i<(int)hexmesh.hexes.size(); ++i) hexes[i] = i;
  std::vector<int> faces(trimesh.faces.size());

  for (int i=0; i<(int)trimesh.faces.size(); ++i) faces[i] = i;

  compute_intersections_KDTree( pr, crosses, trimesh, hexmesh);

  // Flood the two sides.
  pr->update_progress( 0.65 );

  for (int side=0; side<2; side++)
  {
    int start = -1;
    for (int i=0; i<(int)crosses.size(); i++)
    {
      if (crosses[i] < 0)
      {
        start=(int)i;
        break;
      }
    }

    if (start==-1)
    {
      std::cerr<<"couldn't find hex to start flood from!"<<std::endl;
      break;
    }

    std::vector<int> toprocess;
    toprocess.push_back(start);
    crosses[start] = side+1;

    while (toprocess.size())
    {
      VMesh::Elem::index_type h = toprocess.back();
      toprocess.resize(toprocess.size()-1);

      VMesh::Elem::array_type neighbors;
      original_mesh->get_neighbors( neighbors, h );

      if( neighbors.size() > 6 )
        std::cerr << "ERROR: More than six neighbors reported..." << h << std::endl;

      for(size_t i = 0; i < neighbors.size(); i++ )
      {
        VMesh::Elem::index_type hnbr = neighbors[i];
        if (crosses[hnbr] < 0)
        {
          crosses[hnbr] = side+1;
          toprocess.push_back((int)hnbr);
        }
      }
    }
  }

  pr->update_progress( 0.70 );

    //check for non_manifold elements in the input mesh...
  unsigned int k;

  original_mesh->synchronize( Mesh::NODE_NEIGHBORS_E | Mesh::EDGES_E |
                              Mesh::ELEM_NEIGHBORS_E | Mesh::FACES_E );

  bool nmedges_clear = false;
  size_t num_iters = 0; /* inc each time, bail out if it doesn't converge */
  do
  {
    hash_type2 nmedgemap1, nmedgemap2, nmedgemap3;

    for( k = 0; k < crosses.size(); ++k )
    {
      if( add_to_side1 )
      {
        if( crosses[k] == 2 )
        {
            // Get all the faces in the cell.
          VMesh::DElem::array_type faces;
          original_mesh->get_delems( faces, VMesh::Elem::index_type(k) );

            // Check each face for neighbors.
          VMesh::DElem::array_type::iterator fiter = faces.begin();

          while( fiter != faces.end() )
          {
            VMesh::Elem::index_type nci;
            VMesh::DElem::index_type fi = *fiter;
            ++fiter;

            if( original_mesh->get_neighbor( nci, k, fi ) )
            {
              if( crosses[nci] != 2 )
              {
                  //This face is on the boundary of the cut mesh...
//              nmboundary_faces.insert( fi );

                VMesh::Edge::array_type face_edges;
                original_mesh->get_edges( face_edges, fi );
                hash_type2::iterator esearch;
                for( size_t i = 0; i < face_edges.size(); i++ )
                {
                  esearch = nmedgemap1.find( face_edges[i] );
                  if( esearch == nmedgemap1.end() )
                  {
                    nmedgemap1[face_edges[i]] = face_edges[i];
                  }
                  else
                  {
                    esearch = nmedgemap2.find( face_edges[i] );
                    if( esearch == nmedgemap2.end() )
                    {
                      nmedgemap2[face_edges[i]] = face_edges[i];
                    }
                    else
                    {
                      esearch = nmedgemap3.find( face_edges[i] );
                      if( esearch == nmedgemap3.end() )
                      {
                        nmedgemap3[face_edges[i]] = face_edges[i];
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      else
      {
        if( crosses[k] == 1 )
        {
            // Get all the faces in the cell.
          VMesh::DElem::array_type faces;
          original_mesh->get_delems( faces, VMesh::Elem::index_type(k) );

            // Check each face for neighbors.
          VMesh::DElem::array_type::iterator fiter = faces.begin();

          while( fiter != faces.end() )
          {
            VMesh::Elem::index_type nci;
            VMesh::DElem::index_type fi = *fiter;
            ++fiter;

            if( original_mesh->get_neighbor( nci, k, fi ) )
            {
              if( crosses[nci] != 1 )
              {
                  //This face is on the boundary of the cut mesh...
                VMesh::Edge::array_type face_edges;
                original_mesh->get_edges( face_edges, fi );
                hash_type2::iterator esearch;
                for( size_t i = 0; i < face_edges.size(); i++ )
                {
                  esearch = nmedgemap1.find( face_edges[i] );
                  if( esearch == nmedgemap1.end() )
                  {
                    nmedgemap1[face_edges[i]] = face_edges[i];
                  }
                  else
                  {
                    esearch = nmedgemap2.find( face_edges[i] );
                    if( esearch == nmedgemap2.end() )
                    {
                      nmedgemap2[face_edges[i]] = face_edges[i];
                    }
                    else
                    {
                      esearch = nmedgemap3.find( face_edges[i] );
                      if( esearch == nmedgemap3.end() )
                      {
                        nmedgemap3[face_edges[i]] = face_edges[i];
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    if( nmedgemap3.size() != 0 )
    {
      std::cout << "WARNING: New mesh will contain " << nmedgemap3.size() << " non-manifold edges.\n";
      hash_type2::iterator nmsearch = nmedgemap3.begin();
      while( nmsearch != nmedgemap3.end() )
      {
        VMesh::Elem::array_type attached_hexes;
        VMesh::Edge::index_type this_edge = (*nmsearch).first;
        original_mesh->get_elems( attached_hexes, this_edge );

        VMesh::Elem::index_type hex_to_change = 0;
        for(size_t i = 0; i < attached_hexes.size(); i++ )
        {
          VMesh::Elem::index_type hex_id = attached_hexes[i];
          if(hex_id >= static_cast<index_type>(crosses.size()))
          {
            std::cerr << "Invalid Hex ID '" << hex_id << "'; defaulting "
                      << "to 0.  (This is blatantly wrong)" << std::endl;
            hex_id = 0;
          }
//NOTE TO JS1: This is kind of random... We can/should probably be a lot smarter about how we add these....
          if( add_to_side1 )
          {
            if( crosses[hex_id] == 2 )
            {
              hex_to_change = hex_id;
            }
          }
          else
          {
            if( crosses[hex_id] == 1 )
            {
              hex_to_change = hex_id;
            }
          }
        }

        crosses[hex_to_change] = 0;
//end NOTE TO JS1

        ++nmsearch;
      }
    }
    else
    {
      nmedges_clear = true;
    }
    if(++num_iters >= 10) {
      std::cerr << "10 iterations without converging ... bailing out."
                << std::endl;
      break;
    }
  }
  while( nmedges_clear == false );

  // Need to add elements from the three sets of elements.
  hash_type side1_nodemap, side2_nodemap, side1_reverse_map, side2_reverse_map;
  for( k = 0; k < crosses.size(); ++k )
  {
    VMesh::Node::array_type onodes;
    VMesh::Elem::index_type elem_id = k;
    original_mesh->get_nodes( onodes, elem_id );
    VMesh::Node::array_type nnodes(onodes.size());

    if( crosses[k] == 0 )
    {
      if( add_to_side1 )
      {
        for (size_t i = 0; i < onodes.size(); i++)
        {
          if( side1_nodemap.find((index_type)onodes[i]) == side1_nodemap.end())
          {
            Point np;
            original_mesh->get_center( np, onodes[i] );
            const VMesh::Node::index_type nodeindex =
                side1_mesh->add_point( np );
            side1_nodemap[onodes[i]] = nodeindex;
            side1_reverse_map[nodeindex] = onodes[i];
            nnodes[i] = nodeindex;
          }
          else
          {
            nnodes[i] = side1_nodemap[(index_type)onodes[i]];
          }
        }
        side1_mesh->add_elem( nnodes );
      }
      else
      {
        // Add to side2_mesh.
        for (size_t i = 0; i < onodes.size(); i++)
        {
          if( side2_nodemap.find(onodes[i]) == side2_nodemap.end())
          {
            Point np;
            original_mesh->get_center( np, onodes[i] );
            VMesh::Node::index_type nodeindex =
                side2_mesh->add_point( np );
            side2_nodemap[onodes[i]] = nodeindex;
            side2_reverse_map[nodeindex] = onodes[i];
            nnodes[i] = nodeindex;
          }
          else
          {
            nnodes[i] = side2_nodemap[onodes[i]];
          }
        }
        side2_mesh->add_elem( nnodes );
      }
    }
    else if( crosses[k] == 1 )
    {
        // Add to side1_mesh.
      for (size_t i = 0; i < onodes.size(); i++)
      {
        if( side1_nodemap.find(onodes[i]) == side1_nodemap.end())
        {
          Point np;
          original_mesh->get_center( np, onodes[i] );
          const VMesh::Node::index_type nodeindex =
              side1_mesh->add_point( np );
          side1_nodemap[onodes[i]] = nodeindex;
          side1_reverse_map[nodeindex] = onodes[i];
          nnodes[i] = nodeindex;
        }
        else
        {
          nnodes[i] = side1_nodemap[onodes[i]];
        }
      }
      side1_mesh->add_elem( nnodes );
    }
    else
    {
        // Add to side2_mesh.
      for (size_t i = 0; i < onodes.size(); i++)
      {
        if( side2_nodemap.find(onodes[i]) == side2_nodemap.end())
        {
          Point np;
          original_mesh->get_center( np, onodes[i] );
          const VMesh::Node::index_type nodeindex =
              side2_mesh->add_point( np );
          side2_nodemap[onodes[i]] = nodeindex;
          side2_reverse_map[nodeindex] = onodes[i];
          nnodes[i] = nodeindex;
        }
        else
        {
          nnodes[i] = side2_nodemap[onodes[i]];
        }
      }
      side2_mesh->add_elem( nnodes );
    }
  }

  pr->update_progress( 0.75 );
  if( add_layer )
  {
    std::cout << "Adding the new layers of hexes...";
    VMesh::Node::size_type s1_node_size;
    VMesh::Node::size_type s2_node_size;
    hash_type::iterator node_iter;
    std::vector<VMesh::Node::index_type> oi_node_list;
    hash_type shared_vertex_map;
    unsigned int count = 0;

    if( s1_node_size < s2_node_size )
    {
      hash_type::iterator hitr = side1_nodemap.begin();
      while( hitr != side1_nodemap.end() )
      {
        node_iter = side2_nodemap.find( (*hitr).first );
        if( node_iter != side2_nodemap.end() )
        {
          // Want this one.
          oi_node_list.push_back( (*hitr).first );
          count++;
        }
        ++hitr;
      }
    }
    else
    {
      hash_type::iterator hitr = side2_nodemap.begin();
      while( hitr != side2_nodemap.end() )
      {
        node_iter = side1_nodemap.find( (*hitr).first );
        if( node_iter != side1_nodemap.end() )
        {
          // Want this one.
          oi_node_list.push_back( (*hitr).first );
          shared_vertex_map[(*hitr).first] = (*hitr).first;
          count++;
        }
        ++hitr;
      }
    }

    tri_mesh->synchronize( Mesh::FIND_CLOSEST_ELEM_E );
    hash_type new_map1, new_map2;
    size_t i;

    for( i = 0; i < oi_node_list.size(); i++ )
    {
      VMesh::Node::index_type this_node = oi_node_list[i];
      Point n_p;
      original_mesh->get_center( n_p, this_node );

      Point new_result;
      VMesh::Elem::index_type face_id;
      double dist;

      tri_mesh->find_closest_elem( dist, new_result, face_id, n_p );
      Vector dist_vect = 1.5*( new_result - n_p );

        // Finding the closest face can be slow.  Update the progress meter
        // to let the user know that we are performing calculations and the
        // process has not hung.
      if( i%50 == 0 )
      {
        double temp = 0.75 + 0.25*( (double)i/(double)oi_node_list.size() );
        pr->update_progress( temp );
      }

        // Add the new node to the clipped mesh.
      Point new_point( new_result );
      VMesh::Node::index_type this_index1 = side1_mesh->add_point( new_point );
      VMesh::Node::index_type this_index2 = side2_mesh->add_point( new_point );

        // Create a map for the new node to a node on the boundary of
        // the clipped mesh.
      hash_type::iterator node_iter;
      node_iter = new_map1.find( side1_nodemap[this_node] );
      if( node_iter == new_map1.end() )
      {
        new_map1[side1_nodemap[this_node]] = this_index1;
      }
      else
      {
        std::cout << "ERROR\n";
      }

      node_iter = new_map2.find( side2_nodemap[this_node] );
      if( node_iter == new_map2.end() )
      {
        new_map2[side2_nodemap[this_node]] = this_index2;
      }
      else
      {
        std::cout << "ERROR2\n";
      }

      if( add_to_side1 )
      {
        Point p;
        side1_mesh->get_point( p, VMesh::Node::index_type(side1_nodemap[this_node]) );
        double x = p.x(), y = p.y(), z = p.z();
        p.x( dist_vect.x() + x );
        p.y( dist_vect.y() + y );
        p.z( dist_vect.z() + z );
        side1_mesh->set_point( p, VMesh::Node::index_type(side1_nodemap[this_node]) );
      }
      else
      {
        Point p;
        side2_mesh->get_point( p, VMesh::Node::index_type(side2_nodemap[this_node]) );
        double x = p.x(), y = p.y(), z = p.z();
        p.x( dist_vect.x() + x );
        p.y( dist_vect.y() + y );
        p.z( dist_vect.z() + z );
        side2_mesh->set_point( p, VMesh::Node::index_type(side2_nodemap[this_node]) );
      }
    }
    std::cout << "\nFound " << count << " nodes along the shared boundary...\n";

    side1_mesh->synchronize( Mesh::NODE_NEIGHBORS_E | Mesh::EDGES_E | Mesh::ELEM_NEIGHBORS_E | Mesh::FACES_E );
    side2_mesh->synchronize( Mesh::NODE_NEIGHBORS_E | Mesh::EDGES_E | Mesh::ELEM_NEIGHBORS_E | Mesh::FACES_E );

    std::set<VMesh::DElem::index_type> boundary_faces;

      //Walk all the cells in the smallest clipped mesh to find the boundary faces...
    VMesh::Elem::iterator citer; side1_mesh->begin(citer);
    VMesh::Elem::iterator citere; side1_mesh->end(citere);
    hash_type3 face_list;
    hash_type2 edgemap1, edgemap2, edgemap3;

    while( citer != citere )
    {
      VMesh::Elem::index_type ci = *citer;
      ++citer;

        // Get all the faces in the cell.
      VMesh::DElem::array_type faces;
      side1_mesh->get_delems( faces, ci );

        // Check each face for neighbors.
      VMesh::DElem::array_type::iterator fiter = faces.begin();
      VMesh::Node::array_type rnodes(4);

      while( fiter != faces.end() )
      {
        VMesh::Elem::index_type nci;
        VMesh::DElem::index_type fi = *fiter;
        ++fiter;

        if( !side1_mesh->get_neighbor( nci, ci, fi ) )
        {
            // Faces with no neighbors are on the boundary...
            //    make sure that this face isn't on the original boundary

          VMesh::Node::array_type face_nodes;
          side1_mesh->get_nodes( face_nodes, fi );
          hash_type::iterator search1, search2, search3, search4, search_end;
          search_end = shared_vertex_map.end();
          search1 = shared_vertex_map.find( side1_reverse_map[face_nodes[0]] );
          search2 = shared_vertex_map.find( side1_reverse_map[face_nodes[1]] );
          search3 = shared_vertex_map.find( side1_reverse_map[face_nodes[2]] );
          search4 = shared_vertex_map.find( side1_reverse_map[face_nodes[3]] );
          if( search1 != search_end && search2 != search_end &&
              search3 != search_end && search4 != search_end )
          {
            bool ok_to_add_face = true;
            VMesh::DElem::index_type old_face;

            rnodes[0] = VMesh::Node::index_type(side1_reverse_map[face_nodes[0]]);
            rnodes[1] = VMesh::Node::index_type(side1_reverse_map[face_nodes[1]]);
            rnodes[2] = VMesh::Node::index_type(side1_reverse_map[face_nodes[2]]);
            rnodes[3] = VMesh::Node::index_type(side1_reverse_map[face_nodes[3]]);

            if( !original_mesh->get_delem( old_face, rnodes ) )
            {
              std::cout << "ERROR3" << std::endl;
              ok_to_add_face = false;
            }

//NOTE TO JS: Testing (1 - added)...
            boundary_faces.insert( fi );
            face_list[fi] = fi;

              //check for non-manifold edges...
            VMesh::Edge::array_type face_edges;
            side1_mesh->get_edges( face_edges, fi );
            hash_type2::iterator esearch;
            for( i = 0; i < face_edges.size(); i++ )
            {
              esearch = edgemap1.find( face_edges[i] );
              if( esearch == edgemap1.end() )
              {
                edgemap1[face_edges[i]] = face_edges[i];
              }
              else
              {
                esearch = edgemap2.find( face_edges[i] );
                if( esearch == edgemap2.end() )
                {
                  edgemap2[face_edges[i]] = face_edges[i];
                }
                else
                {
                  ok_to_add_face = false;
                  esearch = edgemap3.find( face_edges[i] );
                  if( esearch == edgemap3.end() )
                  {
                    edgemap3[face_edges[i]] = face_edges[i];
                  }
                }
              }
            }

            if( ok_to_add_face )
            {
              hash_type3::iterator test_iter = face_list.find( fi );
              if( test_iter == face_list.end() )
              {
                face_list[fi] = fi;
//NOTE TO JS: Testing (2 - commented out...)
//                  boundary_faces.insert( fi );
              }
            }
          }
        }
      }
    }

      //special casing for projecting faces connected to non-manifold edges...
    if( edgemap3.size() != 0 )
    {
      std::cout << "WARNING: Clipped mesh contains " << edgemap3.size() << " non-manifold edges.\n    Ids are:";
      std::cout << "Boundary has " << boundary_faces.size() << " faces.\n";

      hash_type2::iterator nm_search = edgemap3.begin();
      hash_type2::iterator nm_search_end = edgemap3.end();
      while( nm_search != nm_search_end )
      {
        VMesh::Edge::index_type this_edge = (*nm_search).first;

        bool multiproject_node1 = false;
        bool multiproject_node2 = false;
        VMesh::Node::array_type problem_nodes;
        side1_mesh->get_nodes( problem_nodes, this_edge );
        VMesh::Node::index_type node1 = problem_nodes[0],
            node2 = problem_nodes[1];
        VMesh::Elem::array_type edges_hexes, node1_hexes, node2_hexes;
        side1_mesh->get_elems( edges_hexes, this_edge );
        side1_mesh->get_elems( node1_hexes, node1 );
        side1_mesh->get_elems( node2_hexes, node2 );

        if( node1_hexes.size() == edges_hexes.size() )
        {
          std::cout << " Don't need to multiproject node1 (" << node1 << ")\n";
        }
        else
        {
          std::cout << " Need to multiproject node1 (" << node1 << ")\n";
          multiproject_node1 = true;
        }
        if( node2_hexes.size() == edges_hexes.size() )
        {
          std::cout << " Don't need to multiproject node2 (" << node2 << ")\n";
        }
        else
        {
          std::cout << " Need to multiproject node2 (" << node2 << ")\n";
          multiproject_node2 = true;
        }

        ++nm_search;
      }
      std::cout << std::endl;
    }

    hash_type3::iterator fiter = face_list.begin();
    hash_type3::iterator fiter_end = face_list.end();
    while( fiter != fiter_end )
    {
      VMesh::Node::array_type face_nodes;
      VMesh::DElem::index_type face_id = (*fiter).first;
      side1_mesh->get_nodes( face_nodes, face_id );

      VMesh::Node::array_type nnodes1(8);
      VMesh::Node::array_type nnodes2(8);

      nnodes1[0] = face_nodes[3];
      nnodes1[1] = face_nodes[2];
      nnodes1[2] = face_nodes[1];
      nnodes1[3] = face_nodes[0];

      nnodes2[0] = side2_nodemap[side1_reverse_map[face_nodes[0]]];
      nnodes2[1] = side2_nodemap[side1_reverse_map[face_nodes[1]]];
      nnodes2[2] = side2_nodemap[side1_reverse_map[face_nodes[2]]];
      nnodes2[3] = side2_nodemap[side1_reverse_map[face_nodes[3]]];

      nnodes1[4] = new_map1[face_nodes[3]];
      nnodes1[5] = new_map1[face_nodes[2]];
      nnodes1[6] = new_map1[face_nodes[1]];
      nnodes1[7] = new_map1[face_nodes[0]];

      nnodes2[4] = new_map2[side2_nodemap[side1_reverse_map[face_nodes[0]]]];
      nnodes2[5] = new_map2[side2_nodemap[side1_reverse_map[face_nodes[1]]]];
      nnodes2[6] = new_map2[side2_nodemap[side1_reverse_map[face_nodes[2]]]];
      nnodes2[7] = new_map2[side2_nodemap[side1_reverse_map[face_nodes[3]]]];

      side1_mesh->add_elem( nnodes1 );
      side2_mesh->add_elem( nnodes2 );

      ++fiter;
    }
  }
  std::cout << "Finished\n";

  // Force all the synch data to be rebuilt on next synch call.
  side1_mesh->clear_synchronization();
  side2_mesh->clear_synchronization();

  VMesh::Elem::size_type side1_size;
  VMesh::Elem::size_type side2_size;
  side1_mesh->size( side1_size );
  side2_mesh->size( side2_size );

  std::cout << "Side1 has " << side1_size << " hexes." << std::endl;
  std::cout << "Side2 has " << side2_size << " hexes." << std::endl << std::endl;
  pr->update_progress( 0.99 );
}



class InsertHexVolSheetAlongSurface : public Module
{
  public:
    InsertHexVolSheetAlongSurface(GuiContext* ctx);
    virtual ~InsertHexVolSheetAlongSurface() {}

    virtual void execute();

  private:
    GuiString   add_to_side_;
    GuiString   add_layer_;

    std::string last_add_to_side_;
    std::string last_add_layer_;
};


DECLARE_MAKER(InsertHexVolSheetAlongSurface)

InsertHexVolSheetAlongSurface::InsertHexVolSheetAlongSurface(GuiContext* ctx)
        : Module("InsertHexVolSheetAlongSurface", ctx, Filter, "NewField", "SCIRun"),
          add_to_side_(get_ctx()->subVar("side"), "side1" ),
          add_layer_(get_ctx()->subVar("addlayer"), "On" )
{
}


void
InsertHexVolSheetAlongSurface::execute()
{
  // Get input fields.
  FieldHandle hexfieldhandle;
  FieldHandle trifieldhandle;

  get_input_handle("HexField", hexfieldhandle, true);
  get_input_handle("TriField", trifieldhandle, true);

  if (inputs_changed_ ||
      add_to_side_.changed() ||
      add_layer_.changed() ||
      !oport_cached("Side1Field") ||
      !oport_cached("Side2Field"))
  {
    FieldInformation hfi(hexfieldhandle);
    FieldInformation tfi(trifieldhandle);

    if (!(hfi.is_hex_element()))
    {
      error( "Only Hexhedral field elements are currently supported in the InsertHexVolSheetAlongSurface module.");
      return;
    }

    if (!(tfi.is_tri_element()))
    {
      error( "Only TriSurfFields can be input to the InsertHexVolSheetAlongSurface module.");
      return;
    }


    bool add_to_side1 = false;
    if( add_to_side_.get() == "side1" ) add_to_side1 = true;

    bool add_layer = false;
    if( add_layer_.get() == "On" ) add_layer = true;

    FieldHandle side1field, side2field;
    InsertHexVolSheetAlongSurfaceAlgo algo;

    if(!(algo.execute( this, hexfieldhandle, trifieldhandle,
                   side1field, side2field, add_to_side1, add_layer ))) return;

    send_output_handle("Side1Field", side1field);
    send_output_handle("Side2Field", side2field);
  }
}

} // End namespace SCIRun
