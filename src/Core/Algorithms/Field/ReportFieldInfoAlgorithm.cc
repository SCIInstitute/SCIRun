/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Field/ReportFieldInfoAlgorithm.h>
#include <iostream>

using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun;



struct NumberOfElements : MatrixVisitor
{
  size_t value() const { return value_; }
  
  NumberOfElements() : value_(0) {}
  size_t value_;

  virtual void visit(DenseMatrix& m)
  {
    value_ = m.rows() * m.cols();
  }

  virtual void visit(DenseColumnMatrix& m)
  {
    value_ = m.nrows();
  }

  virtual void visit(SparseRowMatrix& m)
  {
    value_ = m.nonZeros();
  }
};

struct MinimumCoefficient : MatrixVisitor
{
  double value() const { return value_; }

  MinimumCoefficient() : value_(0) {}
  double value_;

  virtual void visit(DenseMatrix& m)
  {
    if (!m.empty())
      value_ = m.minCoeff();
  }

  virtual void visit(DenseColumnMatrix& m)
  {
    if (!m.empty())
      value_ = m.minCoeff();
  }

  virtual void visit(SparseRowMatrix& m)
  {
    if (!m.empty())
      value_ = *std::min_element(m.valuePtr(), m.valuePtr() + m.nonZeros());
  }
};

struct MaximumCoefficient : MatrixVisitor
{
  double value() const { return value_; }

  MaximumCoefficient() : value_(0) {}
  double value_;

  virtual void visit(DenseMatrix& m)
  {
    if (!m.empty())
      value_ = m.maxCoeff();
  }

  virtual void visit(DenseColumnMatrix& m)
  {
     if (!m.empty())
       value_ = m.maxCoeff();
  }

  virtual void visit(SparseRowMatrix& m)
  {
    if (!m.empty())
      value_ = *std::max_element(m.valuePtr(), m.valuePtr() + m.nonZeros());
  }
};



void
ReportFieldInfo::update_input_attributes(FieldHandle f)
{
  VField* vfield = f->vfield();
  VMesh*  vmesh  = f->vmesh();
  
  if (vfield)
  {
    // Get name of field
    std::string fldname;
    if (f->get_property("name",fldname))
    {
      gui_fldname_.set(fldname);
    }
    else
    {
      gui_fldname_.set("--- Name Not Assigned ---");
    }
    
    // Generation
    gui_generation_.set(to_string(f->generation));
    
    // Typename
    const std::string &tname = f->get_type_description()->get_name();
    gui_typename_.set(tname);
    
    // Basis
    static const char *at_table[4] = { "Nodes", "Edges", "Faces", "Cells" };
    switch(f->basis_order())
    {
      case 3:
        gui_dataat_.set("Nodes (cubic basis)");
        break;
      case 2:
        gui_dataat_.set("Nodes (quadratic basis)");
        break;
      case 1:
        gui_dataat_.set("Nodes (linear basis)");
        break;
      case 0:
        gui_dataat_.set(at_table[f->vmesh()->dimensionality()] +
                        std::string(" (constant basis)"));
        break;
      case -1:
        gui_dataat_.set("None (nodata basis)");
        break;
    }
    
    Point center;
    Vector size;
    
    const BBox bbox = vmesh->get_bounding_box();
    if (bbox.valid())
    {
      size = bbox.diagonal();
      center = bbox.center();
      gui_cx_.set(to_string(center.x()));
      gui_cy_.set(to_string(center.y()));
      gui_cz_.set(to_string(center.z()));
      gui_sizex_.set(to_string(size.x()));
      gui_sizey_.set(to_string(size.y()));
      gui_sizez_.set(to_string(size.z()));
      
      size_ = size;
      center_ = center;
    }
    else
    {
      warning("Input Field is empty.");
      gui_cx_.set("--- N/A ---");
      gui_cy_.set("--- N/A ---");
      gui_cz_.set("--- N/A ---");
      gui_sizex_.set("--- N/A ---");
      gui_sizey_.set("--- N/A ---");
      gui_sizez_.set("--- N/A ---");
      
      size_ = Vector(0.0,0.0,0.0);
      center_ = Point(0.0,0.0,0.0);
    }
    
    vfield->minmax(min_,max_);
    gui_datamin_.set(to_string(min_));
    gui_datamax_.set(to_string(max_));
    
    numdata_ = vfield->num_values();
    numnodes_ = vmesh->num_nodes();
    numelems_ = vmesh->num_elems();
    gui_numnodes_.set(to_string(numnodes_));
    gui_numelems_.set(to_string(numelems_));
    gui_numdata_.set(to_string(numdata_));
    
    VMesh::dimension_type dim;
    vmesh->get_dimensions(dim);
    dimensions_ = Vector(1.0,1.0,1.0);
    for (size_t p=0;p<dim.size();p++) dimensions_[p] = static_cast<double>(dim[p]);
    
    gui_nodesx_.set(to_string(dimensions_.x()));
    if (dim.size()>1) gui_nodesy_.set(to_string(dimensions_.y())); else gui_nodesy_.set("-- N/A --");
    if (dim.size()>2) gui_nodesz_.set(to_string(dimensions_.z())); else gui_nodesz_.set("-- N/A --");
    
    geomsize_ = 0.0;
    for (VMesh::Elem::index_type idx=0; idx<numelems_;idx++)
    {
      geomsize_ += vmesh->get_size(idx);
    }
    gui_geomsize_.set(to_string(geomsize_));
    
  }
}

ReportMatrixInfoAlgorithm::Outputs ReportMatrixInfoAlgorithm::run(const Inputs& input, const Parameters& params /* = 0 */) const
{
  ENSURE_ALGORITHM_INPUT_NOT_NULL(input, "Null input field");

  const std::string type = matrix_is::whatType(input);

  NumberOfElements num;
  input->accept(num);
  MinimumCoefficient min;
  input->accept(min);
  MaximumCoefficient max;
  input->accept(max);

  return Outputs(type, 
    input->nrows(), 
    input->ncols(), 
    num.value(),
    min.value(),
    max.value()
    );
}