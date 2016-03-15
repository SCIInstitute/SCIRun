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

#include <Core/Algorithms/Legacy/Fields/StreamLines/GenerateStreamLines.h>
#include <Core/Algorithms/Legacy/Fields/StreamLines/StreamLineIntegrators.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Thread/Interruptible.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

ALGORITHM_PARAMETER_DEF(Fields, StreamlineStepSize);
ALGORITHM_PARAMETER_DEF(Fields, StreamlineTolerance);
ALGORITHM_PARAMETER_DEF(Fields, StreamlineMaxSteps);
ALGORITHM_PARAMETER_DEF(Fields, StreamlineDirection);
ALGORITHM_PARAMETER_DEF(Fields, StreamlineValue);
ALGORITHM_PARAMETER_DEF(Fields, RemoveColinearPoints);
ALGORITHM_PARAMETER_DEF(Fields, StreamlineMethod);
ALGORITHM_PARAMETER_DEF(Fields, AutoParameters);
ALGORITHM_PARAMETER_DEF(Fields, NumStreamlines);

GenerateStreamLinesAlgo::GenerateStreamLinesAlgo()
{
  addParameter(Parameters::StreamlineStepSize, 0.01);
  addParameter(Parameters::StreamlineTolerance, 0.0001);
  addParameter(Parameters::StreamlineMaxSteps, 2000);
  addOption(Parameters::StreamlineDirection, "Both", "Negative|Both|Positive");
  addOption(Parameters::StreamlineValue, "Seed index", "Seed value|Seed index|Integration index|Integration step|Distance from seed|Streamline length");
  addParameter(Parameters::RemoveColinearPoints, false);
  addOption(Parameters::StreamlineMethod, "RungeKuttaFehlberg", "AdamsBashforth|Heun|RungeKutta|RungeKuttaFehlberg|CellWalk");
  // Estimate step size and tolerance automatically based on average edge length
  addParameter(Parameters::AutoParameters,false);

  // For output
  addParameter(Parameters::NumStreamlines, 0);
}


namespace detail
{

void CleanupStreamLinePoints(const std::vector<Point> &input, std::vector<Point> &output, double e2)
{
  // Removes colinear points from the list of points.
  size_t i, j = 0;

  if (input.size())
  {
    output.push_back(input[0]);

    for (i=1; i < input.size(); i++)
    {
      const Vector v0 = input[i-1] - output[j];
      const Vector v1 = input[i] - input[i-1];

      if (Cross(v0, v1).length2() > 1e10*e2)
      {
        j++; output.push_back(input[i]);
      }
    }
  }
}

bool directionIncludesNegative(int dir)
{
  return dir <= 1;
}

bool directionIncludesPositive(int dir)
{
  return dir >= 1;
}

bool directionIsBoth(int dir)
{
  return 1 == dir;
}

int convertDirectionOption(const std::string& dir)
{
  if (dir == "Negative")
    return 0;
  else if (dir == "Both")
    return 1;
  else // Positive
    return 2;
}

IntegrationMethod convertMethod(const std::string& option)
{
  int method = 0;
  if (option == "AdamsBashforth") method = 0;
  else if (option == "Heun") method = 2;
  else if (option == "RungeKutta") method = 3;
  else if (option == "RungeKuttaFehlberg") method = 4;
  else if (option == "CellWalk") method = 5;
  else
    BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Unknown streamline method selected"));
  return IntegrationMethod(method);
}

StreamlineValue convertValue(const std::string& option)
{
  //"Seed value|Seed index|Integration index|Integration step|Distance from seed|Streamline length"
  if (option == "Seed value")
    return SeedValue;
  if (option == "Seed index")
    return SeedIndex;
  if (option == "Integration index")
    return IntegrationIndex;
  if (option == "Integration step")
    return IntegrationStep;
  if (option == "Distance from seed")
    return DistanceFromSeed;
  if (option == "Streamline length")
    return StreamlineLength;

  BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Unknown streamline value selected"));
}


class GenerateStreamLinesAlgoP : public Core::Thread::Interruptible
{

  public:
    GenerateStreamLinesAlgoP() :
      tolerance_(0), step_size_(0), max_steps_(0), direction_(0), value_(SeedIndex), remove_colinear_pts_(false),
      method_(AdamsBashforth), seed_field_(0), seed_mesh_(0), field_(0), mesh_(0), ofield_(0), omesh_(0), algo_(0), success_(false)
    {}

    bool run(const AlgorithmBase* algo, FieldHandle input,
             FieldHandle seeds, FieldHandle& output,
             IntegrationMethod method);

  private:
    void runImpl();

    double tolerance_;
    double step_size_;
    int    max_steps_;
    int    direction_;
    StreamlineValue    value_;
    bool   remove_colinear_pts_;
    IntegrationMethod method_;

    VField* seed_field_;
    VMesh*  seed_mesh_;

    VField* field_;
    VMesh*  mesh_;

    VField* ofield_;
    VMesh*  omesh_;

    const AlgorithmBase* algo_;

    bool success_;
};


void
GenerateStreamLinesAlgoP::runImpl()
{
  // first candidate for module/algo stopping.
  try
  {
    VMesh::Node::index_type n1, n2;
    Vector test;

    StreamLineIntegrators BI;
    BI.nodes_.reserve(max_steps_);                  // storage for points
    BI.tolerance2_  = tolerance_ * tolerance_;      // square error tolerance
    BI.max_steps_    = max_steps_;                  // max number of steps
    BI.vfield_      = field_;                       // the vector field
    std::vector<Point>::iterator node_iter;

    // Try to find the streamline for each seed point.
    VMesh::size_type num_seeds = seed_mesh_->num_nodes();
    VMesh::Node::array_type newnodes(2);

    for (VMesh::Node::index_type idx=1; idx<num_seeds; ++idx)
    {
      checkForInterruption();

      seed_mesh_->get_point(BI.seed_, idx);

       // Is the seed point inside the field?
      if (!field_->interpolate(test, BI.seed_))
        continue;

      BI.nodes_.clear();
      BI.nodes_.push_back(BI.seed_);

      int cc = 0;

      // Find the negative streamlines.
      if (directionIncludesNegative(direction_))
      {
        BI.step_size_ = -step_size_;   // initial step size
        BI.integrate( method_ );

        if (directionIsBoth(direction_))
        {
          BI.seed_ = BI.nodes_[0];     // Reset the seed

          reverse(BI.nodes_.begin(), BI.nodes_.end());
          cc = BI.nodes_.size() - 1;
          cc = -(cc - 1);
        }
      }

      // Append the positive streamlines.
      if (directionIncludesPositive(direction_))
      {
        BI.step_size_ = step_size_;   // initial step size
        BI.integrate( method_ );
      }

      double length = 0;
      Point p1;

      if (value_ == StreamlineLength)
      {
        node_iter = BI.nodes_.begin();
        if (node_iter != BI.nodes_.end())
        {
          p1 = *node_iter;
          ++node_iter;

          while (node_iter != BI.nodes_.end())
          {
            length += Vector( *node_iter-p1 ).length();
            p1 = *node_iter;
            ++node_iter;
          }
        }
      }

      node_iter = BI.nodes_.begin();

      if (node_iter != BI.nodes_.end())
      {
        p1 = *node_iter;
        n1 = omesh_->add_point(p1);

	// Record the streamline point indexes. Used downstream.
	//std::ostringstream str;
	//str << "Streamline " << (unsigned int) idx << " Node Index";
	//ofield_->set_property( str.str(), (unsigned int) n1, false );

        ofield_->resize_values();

        if (value_ == SeedValue) ofield_->copy_value(seed_field_, idx, n1);
        else if (value_ == SeedIndex) ofield_->set_value(index_type(idx), n1);
        else if (value_ == IntegrationIndex) ofield_->set_value(abs(cc), n1);
        else if (value_ == IntegrationStep) ofield_->set_value(0, n1);
        else if (value_ == DistanceFromSeed) ofield_->set_value(0, n1);
        else if (value_ == StreamlineLength) ofield_->set_value(length, n1);

        ++node_iter;
        cc++;

        while (node_iter != BI.nodes_.end())
        {
          n2 = omesh_->add_point(*node_iter);
          ofield_->resize_fdata();

          if (value_ == SeedValue) ofield_->copy_value(seed_field_, idx, n2);
          else if (value_ == SeedIndex) ofield_->set_value(index_type(idx), n2);
          else if (value_ == IntegrationIndex) ofield_->set_value(abs(cc), n2);
          else if (value_ == IntegrationStep)
          {
            length = Vector( *node_iter-p1 ).length();
            ofield_->set_value(length,n2);
          }
          else if (value_ == DistanceFromSeed)
          {
            length += Vector( *node_iter-p1 ).length();
            ofield_->set_value(length,n2);
          }
          else if (value_ == StreamlineLength)
	        {
	           ofield_->set_value(length,n2);
          }

          newnodes[0] = n1;
          newnodes[1] = n2;

          omesh_->add_elem(newnodes);
          n1 = n2;
          ++node_iter;

          cc++;
        }
      }

	    algo_->update_progress_max(idx,num_seeds);
    }

    #ifdef NEEDS_ADDITIONAL_ALGO_OUTPUT
    algo_->set_int("num_streamlines", num_seeds);
    #endif

    // Record the number of streamline. Used downstream.
    //ofield_->set_property( "Streamline Count", (unsigned int) num_seeds, false );
   }

  catch (const Exception &e)
  {
    algo_->error(std::string("Crashed with the following exception:\n")+e.message());
    success_ = false;
  }
  catch (const std::string& a)
  {
    algo_->error(a);
    success_ = false;
  }
  catch (const char *a)
  {
    algo_->error(a);
    success_ = false;
  }
}


bool
GenerateStreamLinesAlgoP::run(const AlgorithmBase* algo,
                              FieldHandle input,
                              FieldHandle seeds,
                              FieldHandle& output,
                              IntegrationMethod method)
{
  seed_field_ = seeds->vfield();
  seed_mesh_ = seeds->vmesh();
  field_ = input->vfield();
  mesh_ = input->vmesh();
  ofield_ = output->vfield();
  omesh_ = output->vmesh();
  algo_ = algo;

  tolerance_ = algo->get(Parameters::StreamlineTolerance).toDouble();
  step_size_ = algo->get(Parameters::StreamlineStepSize).toDouble();
  max_steps_ = algo->get(Parameters::StreamlineMaxSteps).toInt();
  direction_ = convertDirectionOption(algo->getOption(Parameters::StreamlineDirection));
  value_ = convertValue(algo->getOption(Parameters::StreamlineValue));
  remove_colinear_pts_ = algo->get(Parameters::RemoveColinearPoints).toBool();
  method_ = method;

  success_ = true;

  runImpl();

  return (success_);
}


// Cell walk streamline code



class GenerateStreamLinesAccAlgo {

  public:
    GenerateStreamLinesAccAlgo() :
      max_steps_(0), direction_(0), value_(SeedIndex), remove_colinear_pts_(false),
      seed_field_(0), seed_mesh_(0), field_(0), mesh_(0), ofield_(0),
      omesh_(0), success_(false)
      {}

    bool run(const AlgorithmBase* algo, FieldHandle input,
             FieldHandle seeds, FieldHandle& output);

    void find_nodes(std::vector<Point>& v, Point seed, bool back);
  private:
    int    max_steps_;
    int    direction_;
    StreamlineValue    value_;
    bool   remove_colinear_pts_;

    VField* seed_field_;
    VMesh*  seed_mesh_;

    VField* field_;
    VMesh*  mesh_;

    VField* ofield_;
    VMesh*  omesh_;

    bool success_;
};

bool
GenerateStreamLinesAccAlgo::run(const AlgorithmBase* algo,
				FieldHandle input,
				FieldHandle seeds,
				FieldHandle& output)
{
  success_ = true;
  try
  {
    seed_field_ = seeds->vfield();
    seed_mesh_ = seeds->vmesh();
    field_ = input->vfield();
    mesh_ = input->vmesh();
    ofield_ = output->vfield();
    omesh_ = output->vmesh();

    max_steps_ = algo->get(Parameters::StreamlineMaxSteps).toInt();
    direction_ = convertDirectionOption(algo->getOption(Parameters::StreamlineDirection));
    value_ = convertValue(algo->getOption(Parameters::StreamlineValue));
    remove_colinear_pts_ = algo->get(Parameters::RemoveColinearPoints).toBool();

    Point seed;

    VMesh::Elem::index_type elem;
    std::vector<Point> nodes;
    nodes.reserve(max_steps_);

    std::vector<Point>::iterator node_iter;
    VMesh::Node::index_type n1, n2;
    VMesh::Node::array_type newnodes(2);

    // Try to find the streamline for each seed point.
    VMesh::size_type num_seeds = seed_mesh_->num_nodes();

    for(VMesh::Node::index_type idx=0; idx<num_seeds; ++idx)
    {
      seed_mesh_->get_center(seed, idx);

      // Is the seed point inside the field?
      if (!(mesh_->locate(elem, seed)))
        continue;
      nodes.clear();
      nodes.push_back(seed);

      int cc = 0;

      // Find the negative streamlines.
      if (directionIncludesNegative(direction_))
      {
        find_nodes(nodes, seed, true);

        if (directionIsBoth(direction_))
        {
          std::reverse(nodes.begin(), nodes.end());
          cc = nodes.size();
          cc = -(cc - 1);
        }
      }

      // Append the positive streamlines.
      if (directionIncludesPositive(direction_))
      {
        find_nodes(nodes, seed, false);
      }

      double length = 0;
      Point p1;

      if (value_ == StreamlineLength)
      {
        node_iter = nodes.begin();
        if (node_iter != nodes.end())
        {
          p1 = *node_iter;
          ++node_iter;

          while (node_iter != nodes.end())
          {
            length += Vector( *node_iter-p1 ).length();
            p1 = *node_iter;
            ++node_iter;
          }
        }
      }

      node_iter = nodes.begin();

      if (node_iter != nodes.end())
      {
        p1 = *node_iter;
        n1 = omesh_->add_point(*node_iter);

	// Record the streamline point indexes. Used downstream.
	//std::ostringstream str;
	//str << "Streamline " << (unsigned int) idx << " Node Index";
	//ofield_->set_property( str.str(), (unsigned int) n1, false );

        ofield_->resize_values();

        if (value_ == SeedValue) ofield_->copy_value(field_, idx, n1);
        else if (value_ == SeedIndex) ofield_->set_value(static_cast<int>(idx), n1);
        else if (value_ == IntegrationIndex) ofield_->set_value(abs(cc), n1);
        else if (value_ == IntegrationStep) ofield_->set_value(0, n1);
        else if (value_ == DistanceFromSeed) ofield_->set_value(0, n1);
        else if (value_ == StreamlineLength) ofield_->set_value(length, n1);
        ++node_iter;

        cc++;

        while (node_iter != nodes.end())
        {
          n2 = omesh_->add_point(*node_iter);
          ofield_->resize_values();

          if (value_ == SeedValue) ofield_->copy_value(field_, idx, n2);
          else if (value_ == SeedIndex) ofield_->set_value(static_cast<int>(idx), n2);
          else if (value_ == IntegrationIndex) ofield_->set_value(abs(cc), n2);
          else if (value_ == IntegrationStep)
          {
            length = Vector( *node_iter-p1 ).length();
            ofield_->set_value(length,n2);
          }
          else if (value_ == DistanceFromSeed)
          {
            length += Vector( *node_iter-p1 ).length();
            ofield_->set_value(length,n2);
          }
          else if (value_ == StreamlineLength)
	       {
	          ofield_->set_value(length,n2);
          }

          newnodes[0] = n1;
          newnodes[1] = n2;

          omesh_->add_elem(newnodes);

          n1 = n2;
          ++node_iter;

          cc++;
        }
      }

      algo->update_progress_max(idx, num_seeds);
    }

#ifdef NEED_ADDITIONAL_ALGO_OUTPUT
    algo->set_int("num_streamlines", num_seeds);
    #endif

    // Record the number of streamline. Used downstream.
    //ofield_->set_property( "Streamline Count", (unsigned int) num_seeds, false );
  }

  catch (const Exception &e)
  {
    algo->error(std::string("Crashed with the following exception:\n")+e.message());
    success_ = false;
  }
  catch (const std::string& a)
  {
    algo->error(a);
    success_ = false;
  }
  catch (const char *a)
  {
    algo->error(a);
    success_ = false;
  }

  return (success_);
}


void GenerateStreamLinesAccAlgo::find_nodes(std::vector<Point> &v, Point seed, bool back)
{
  VMesh::Elem::index_type elem, neighbor;
  VMesh::Face::array_type faces;
  VMesh::Node::array_type nodes;
  VMesh::Face::index_type minface;
  VMesh::Face::index_type lastface;
  Vector lastnormal(0,0,0), minnormal(0,0,0);
  Vector dir;
  std::vector<Point> points(3);
  std::vector<Point> tv;

  if (!(mesh_->locate(elem, seed)))
  {
    return;
  }

  lastface = -1;

  tv.push_back(seed);

  for (int i=0; i < max_steps_; i++)
  {
    field_->get_value(dir, elem);
    dir.safe_normalize();
    if (back) { dir *= -1.0; }

    if (i && (Dot(dir, lastnormal) < 1e-12))
    {
      dir = dir - lastnormal * Dot(dir, lastnormal);
      if (dir.safe_normalize() < 1.0e-6)
        break;
    }

    mesh_->get_faces(faces, elem);
    double mindist = DBL_MAX;
    bool found = false;
    Point ecenter;

    mesh_->get_center(ecenter, elem);
    for (size_t j=0; j < faces.size(); j++)
    {
      // Do not check last face as our see point is already on top of it
      if (faces[j] == lastface)
        continue;

      mesh_->get_nodes(nodes, faces[j]);
      mesh_->get_centers(points, nodes);
      Vector normal = Cross(points[1]-points[0], points[2]-points[0]);
      normal.safe_normalize();
      if (Dot(normal, ecenter-points[0]) > 0.0)
      {
        normal *= -1.0;
      }

      const double Vd = Dot(dir, normal);
      if (Vd < 1e-12) continue;
      const double V0 = Dot(normal, (points[0]-seed));
      const double dist = V0/Vd;
      if (dist > -1e-12 && dist < mindist)
      {
        mindist = dist;
        minface = faces[j];
        minnormal = normal;
        found = true;
      }
    }

    if (!found)
      break;

    seed = seed + dir * mindist;

    tv.push_back(seed);
    if (!(mesh_->get_neighbor(neighbor, elem, VMesh::DElem::index_type(minface))))
      break;

    elem = neighbor;
    lastnormal = minnormal;
    lastface = minface;
  }

  if (remove_colinear_pts_)
  {
    CleanupStreamLinePoints(tv,v, mesh_->get_epsilon()*mesh_->get_epsilon());
  }
}

} // end namespace detail

bool GenerateStreamLinesAlgo::runImpl(FieldHandle input, FieldHandle seeds, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "GenerateStreamLines");

  if (!input)
  {
    error("No input source field");
    return (false);
  }

  if (!seeds)
  {
    error("No input seed points");
    return (false);
  }

  VField* ifield = input->vfield();
  VMesh* mesh = input->vmesh();

  if (!(ifield->is_vector()))
  {
    error("Input field is not a Vector field.");
    return (false);
  }

  if (mesh->dimensionality() < 2)
  {
    error("This algorithm only works for volumes or surfaces");
    return (false);
  }

  auto method = detail::convertMethod(getOption(Parameters::StreamlineMethod));

  if (method == CellWalk && ifield->basis_order() != 0)
  {
    error("The Cell Walk method only works for cell centered Vector Fields.");
    return (false);
  }

  FieldInformation fi(input);
  fi.make_curvemesh();
  fi.make_lineardata();
  fi.make_linearmesh();
  fi.make_double();

  output = CreateField(fi);

  if (!output)
  {
    error("Could not create output field");
    return (false);
  }

  const bool autoParams = get(Parameters::AutoParameters).toBool();
  if (autoParams)
  {
    mesh->synchronize(Mesh::EPSILON_E|Mesh::ELEM_LOCATE_E|Mesh::EDGES_E|Mesh::FACES_E);
  }
  else
  {
    mesh->synchronize(Mesh::EPSILON_E|Mesh::ELEM_LOCATE_E|Mesh::FACES_E);
  }

  bool success = false;

  if (method == 5)
  {
    detail::GenerateStreamLinesAccAlgo algo;
    success = algo.run(this,input,seeds,output);
  }
  else
  {
    if (autoParams)
    {
      VMesh::size_type num_edges = mesh->num_edges();
      double length = 0;

      for (VMesh::Edge::index_type idx=0; idx<num_edges;idx++)
      {
        length += mesh->get_size(idx);
      }
      length = length / num_edges;

    #ifdef NEED_ADDITIONAL_OUTPUT_OBJECT
      set_scalar("tolerance",length/20.0);
      set_scalar("step_size",length/5.0);
    #endif
    }

    detail::GenerateStreamLinesAlgoP algo;
    success = algo.run(this,input,seeds,output,method);
  }

  return (success);
}

const AlgorithmInputName GenerateStreamLinesAlgo::VectorField("Vector_Field");
const AlgorithmInputName GenerateStreamLinesAlgo::Seeds("Seed_Points");
const AlgorithmOutputName GenerateStreamLinesAlgo::Streamlines("Streamlines");

AlgorithmOutput GenerateStreamLinesAlgo::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(VectorField);
  auto seeds = input.get<Field>(Seeds);
  FieldHandle outputField;

  if (!runImpl(field, seeds, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");
  AlgorithmOutput output;
  output[Streamlines] = outputField;
  return output;
}
