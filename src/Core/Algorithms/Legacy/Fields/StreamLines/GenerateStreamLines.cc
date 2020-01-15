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

   Extension:     Thread-based parallelization
   Author:        Moritz Dannhauer
   Date:          August 2017
*/


#include <Core/Algorithms/Legacy/Fields/StreamLines/GenerateStreamLines.h>
#include <Core/Algorithms/Legacy/Fields/StreamLines/StreamLineIntegrators.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/Fields/MergeFields/JoinFieldsAlgo.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Thread/Interruptible.h>
#include <Core/Thread/Barrier.h>
#include <Core/Thread/Parallel.h>
#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Thread;

ALGORITHM_PARAMETER_DEF(Fields, StreamlineStepSize);
ALGORITHM_PARAMETER_DEF(Fields, StreamlineTolerance);
ALGORITHM_PARAMETER_DEF(Fields, StreamlineMaxSteps);
ALGORITHM_PARAMETER_DEF(Fields, StreamlineDirection);
ALGORITHM_PARAMETER_DEF(Fields, StreamlineValue);
ALGORITHM_PARAMETER_DEF(Fields, RemoveColinearPoints);
ALGORITHM_PARAMETER_DEF(Fields, StreamlineMethod);
ALGORITHM_PARAMETER_DEF(Fields, AutoParameters);
ALGORITHM_PARAMETER_DEF(Fields, NumStreamlines);
ALGORITHM_PARAMETER_DEF(Fields, UseMultithreading);

GenerateStreamLinesAlgo::GenerateStreamLinesAlgo()
{
  addParameter(Parameters::StreamlineStepSize, 0.01);
  addParameter(Parameters::StreamlineTolerance, 0.0001);
  addParameter(Parameters::StreamlineMaxSteps, 2000);
  addOption(Parameters::StreamlineDirection, "Both", "Negative|Both|Positive");
  addOption(Parameters::StreamlineValue, "Seed index", "Seed value|Seed index|Integration index|Integration step|Distance from seed|Streamline length");
  addParameter(Parameters::RemoveColinearPoints, true);
  addOption(Parameters::StreamlineMethod, "RungeKuttaFehlberg", "AdamsBashforth|Heun|RungeKutta|RungeKuttaFehlberg|CellWalk");
  // Estimate step size and tolerance automatically based on average edge length
  addParameter(Parameters::AutoParameters, false);

  // For output
  addParameter(Parameters::NumStreamlines, 0);

  addParameter(Parameters::UseMultithreading, true);
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

      for (i = 1; i < input.size(); i++)
      {
        const Vector v0 = input[i - 1] - output[j];
        const Vector v1 = input[i] - input[i - 1];

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
    if (option == "AdamsBashforth")
      return IntegrationMethod::AdamsBashforth;
    if (option == "Heun")
      return IntegrationMethod::Heun;
    if (option == "RungeKutta")
      return IntegrationMethod::RungeKutta;
    if (option == "RungeKuttaFehlberg")
      return IntegrationMethod::RungeKuttaFehlberg;
    if (option == "CellWalk")
      return IntegrationMethod::CellWalk;

    BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Unknown streamline method selected: " + option));
  }

  StreamlineValue convertValue(const std::string& option)
  {
    //"Seed value|Seed index|Integration index|Integration step|Distance from seed|Streamline length"
    if (option == "Seed value")
      return StreamlineValue::SeedValue;
    if (option == "Seed index")
      return StreamlineValue::SeedIndex;
    if (option == "Integration index")
      return StreamlineValue::IntegrationIndex;
    if (option == "Integration step")
      return StreamlineValue::IntegrationStep;
    if (option == "Distance from seed")
      return StreamlineValue::DistanceFromSeed;
    if (option == "Streamline length")
      return StreamlineValue::StreamlineLength;

    BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Unknown streamline value selected"));
  }

  class GenerateStreamLinesAlgoImplBase : public Core::Thread::Interruptible
  {
  public:
    GenerateStreamLinesAlgoImplBase(const AlgorithmBase* algo, IntegrationMethod method) : algo_(algo),
      numprocessors_(Parallel::NumCores()),
      barrier_("GenerateStreamLinesAlgoImplBase Barrier", numprocessors_), method_(method)
    {}

    bool run(FieldHandle input, FieldHandle seeds, FieldHandle& output);

    std::pair<index_type, index_type> partitionNodes(int proc_num) const
    {
      const index_type start_gd = (global_dimension_ * proc_num) / numprocessors_;
      const index_type end_gd = (global_dimension_ * (proc_num + 1)) / numprocessors_;

      LOG_DEBUG("GenerateStreamLinesAlgoP proc {}, start {}, end {}", proc_num, start_gd, end_gd);
      return {start_gd, end_gd};
    }

  protected:
    void parallel(int proc);
    virtual FieldHandle StreamLinesForCertainSeeds(VMesh::Node::index_type from, VMesh::Node::index_type to, int proc_num) = 0;
    double calcTotalStreamlineLength(const std::vector<Point>& nodes) const;
    void setOutputData(FieldHandle out, const std::vector<Point>& nodes, VMesh::Node::index_type idx, int cc) const;

    const AlgorithmBase* algo_;
    int numprocessors_;
    Barrier barrier_;
    double tolerance_ {0};
    double step_size_ {0};
    int    max_steps_ {0};
    int    direction_ {0};
    StreamlineValue    value_ {StreamlineValue::SeedIndex};
    bool   remove_colinear_pts_ {false};
    IntegrationMethod method_;

    VField* seed_field_ {nullptr};
    VMesh*  seed_mesh_ {nullptr};

    VField* field_ {nullptr};
    VMesh*  mesh_ {nullptr};

    FieldHandle input_;
    std::vector<bool> success_;
    FieldList outputs_;
    VMesh::Node::index_type global_dimension_ {0};
  };

  double GenerateStreamLinesAlgoImplBase::calcTotalStreamlineLength(const std::vector<Point>& nodes) const
  {
    double totalStreamlineLength = 0;
    if (value_ == StreamlineValue::StreamlineLength)
    {
      if (nodes.size() > 1)
      {
        for (size_t i = 1; i < nodes.size(); ++i)
        {
          totalStreamlineLength += Vector(nodes[i - 1] - nodes[i]).length();
        }
      }
    }
    return totalStreamlineLength;
  }

  class GenerateStreamLinesAlgoP : public GenerateStreamLinesAlgoImplBase
  {

  public:
    GenerateStreamLinesAlgoP(const AlgorithmBase* algo, IntegrationMethod method) : GenerateStreamLinesAlgoImplBase(algo, method)
    {}
  protected:
    FieldHandle StreamLinesForCertainSeeds(VMesh::Node::index_type from, VMesh::Node::index_type to, int proc_num) override;
  };

  FieldHandle GenerateStreamLinesAlgoP::StreamLinesForCertainSeeds(VMesh::Node::index_type from, VMesh::Node::index_type to, int proc_num)
  {
    FieldHandle out;
    try
    {
      Vector test;
      FieldInformation fi(input_);
      fi.make_curvemesh();
      fi.make_lineardata();
      fi.make_linearmesh();
      fi.make_double();
      out = CreateField(fi);

      StreamLineIntegrators BI;
      BI.nodes_.reserve(max_steps_);                  // storage for points
      BI.tolerance2_ = tolerance_ * tolerance_;      // square error tolerance
      BI.max_steps_ = max_steps_;                  // max number of steps
      BI.vfield_ = field_;                       // the vector field
      std::vector<Point>::iterator node_iter;

      // Try to find the streamline for each seed point.
      for (VMesh::Node::index_type idx = from; idx < to; ++idx)
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
          BI.integrate(method_);

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
          BI.integrate(method_);
        }

        setOutputData(out, BI.nodes_, idx, cc);

        if (proc_num == 0)
          algo_->update_progress_max(idx, to);
      }

#ifdef NEEDS_ADDITIONAL_ALGO_OUTPUT
      algo_->set_int("num_streamlines", num_seeds);
#endif
    }

    catch (const Exception &e)
    {
      algo_->error(std::string("Crashed with the following exception:\n") + e.message());
      success_[proc_num] = false;
    }
    catch (const std::string& a)
    {
      algo_->error(a);
      success_[proc_num] = false;
    }
    catch (const char *a)
    {
      algo_->error(a);
      success_[proc_num] = false;
    }

    return out;
  }

  void GenerateStreamLinesAlgoImplBase::setOutputData(FieldHandle out, const std::vector<Point>& nodes, VMesh::Node::index_type idx, int cc) const
  {
    auto ofield = out->vfield();
    auto omesh = out->vmesh();
    const auto totalLength = calcTotalStreamlineLength(nodes);
    double partialStreamlineLength = 0;
    int nodeIndex = 0;
    Point previousNode;
    VMesh::Node::array_type newnodes(2);
    VMesh::Node::index_type n1, n2;

    for (const auto& node : nodes)
    {
      if (0 == nodeIndex)
      {
        n1 = omesh->add_point(node);
        ofield->resize_values();

        if (value_ == StreamlineValue::SeedValue) ofield->copy_value(seed_field_, idx, n1);
        else if (value_ == StreamlineValue::SeedIndex) ofield->set_value(index_type(idx), n1);
        else if (value_ == StreamlineValue::IntegrationIndex) ofield->set_value(abs(cc), n1);
        else if (value_ == StreamlineValue::IntegrationStep) ofield->set_value(0, n1);
        else if (value_ == StreamlineValue::DistanceFromSeed) ofield->set_value(partialStreamlineLength, n1);
        else if (value_ == StreamlineValue::StreamlineLength) ofield->set_value(totalLength, n1);

        cc++;
      }
      else
      {
        n2 = omesh->add_point(node);
        ofield->resize_fdata();

        if (value_ == StreamlineValue::SeedValue) ofield->copy_value(seed_field_, idx, n2);
        else if (value_ == StreamlineValue::SeedIndex) ofield->set_value(index_type(idx), n2);
        else if (value_ == StreamlineValue::IntegrationIndex) ofield->set_value(abs(cc), n2);
        else if (value_ == StreamlineValue::IntegrationStep)
        {
          double length = Vector(node - previousNode).length();
          ofield->set_value(length, n2);
        }
        else if (value_ == StreamlineValue::DistanceFromSeed)
        {
          partialStreamlineLength += Vector(node - previousNode).length();
          ofield->set_value(partialStreamlineLength, n2);
        }
        else if (value_ == StreamlineValue::StreamlineLength)
        {
          ofield->set_value(totalLength, n2);
        }

        newnodes[0] = n1;
        newnodes[1] = n2;

        omesh->add_elem(newnodes);
        n1 = n2;

        cc++;
      }
      ++nodeIndex;
      previousNode = node;
    }
  }

  void GenerateStreamLinesAlgoImplBase::parallel(int proc_num)
  {
    success_[proc_num] = true;

    for (int q = 0; q < numprocessors_; q++)
    {
      if (!success_[q])
        return;
    }

    auto range = partitionNodes(proc_num);
    outputs_[proc_num] = StreamLinesForCertainSeeds(range.first, range.second, proc_num);
  }

  bool GenerateStreamLinesAlgoImplBase::run(FieldHandle input,
    FieldHandle seeds,
    FieldHandle& output)
  {
    input_ = input;
    seed_field_ = seeds->vfield();
    seed_mesh_ = seeds->vmesh();
    field_ = input->vfield();
    mesh_ = input->vmesh();
    tolerance_ = algo_->get(Parameters::StreamlineTolerance).toDouble();
    step_size_ = algo_->get(Parameters::StreamlineStepSize).toDouble();
    max_steps_ = algo_->get(Parameters::StreamlineMaxSteps).toInt();
    direction_ = convertDirectionOption(algo_->getOption(Parameters::StreamlineDirection));
    value_ = convertValue(algo_->getOption(Parameters::StreamlineValue));
    remove_colinear_pts_ = algo_->get(Parameters::RemoveColinearPoints).toBool();
    global_dimension_ = seed_mesh_->num_nodes();
    if (global_dimension_ < numprocessors_ || numprocessors_ < 1) numprocessors_ = 1;
    if (numprocessors_ > 16)
      numprocessors_ = 16;  // limit the number of threads
    if (!algo_->get(Parameters::UseMultithreading).toBool())
      numprocessors_ = 1;
    success_.resize(numprocessors_, true);
    outputs_.resize(numprocessors_, nullptr);

    Parallel::RunTasks([this](int i) { parallel(i); }, numprocessors_);
    for (size_t j = 0; j < success_.size(); j++)
    {
      if (!success_[j]) return false;
      if (!outputs_[j]) return false;
    }
    JoinFieldsAlgo join;
    join.set(JoinFieldsAlgo::MergeNodes, false);
    if (IntegrationMethod::CellWalk == method_)
      join.set(JoinFieldsAlgo::Tolerance, 1e-8);
    join.runImpl(outputs_, output);

    return true;
  }

  // Cell walk streamline code
  class GenerateStreamLinesAccAlgo : public GenerateStreamLinesAlgoImplBase
  {

  public:
    GenerateStreamLinesAccAlgo(const AlgorithmBase* algo, IntegrationMethod method) : GenerateStreamLinesAlgoImplBase(algo, method)
    {}
  protected:
    FieldHandle StreamLinesForCertainSeeds(VMesh::Node::index_type from, VMesh::Node::index_type to, int proc_num) override;
  private:
    void find_nodes(std::vector<Point>& v, Point seed, bool back);
  };

  FieldHandle GenerateStreamLinesAccAlgo::StreamLinesForCertainSeeds(VMesh::Node::index_type from, VMesh::Node::index_type to, int proc_num)
  {
    FieldInformation fi(input_);
    fi.make_curvemesh();
    fi.make_lineardata();
    fi.make_linearmesh();
    fi.make_double();
    FieldHandle out;
    try
    {
      out = CreateField(fi);
      Point seed;
      VMesh::Elem::index_type elem;
      std::vector<Point> nodes;
      nodes.reserve(max_steps_);

      // Try to find the streamline for each seed point.
      for (VMesh::Node::index_type idx = from; idx < to; ++idx)
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

        setOutputData(out, nodes, idx, cc);

        if (proc_num == 0)
          algo_->update_progress_max(from, to);
      }

#ifdef NEED_ADDITIONAL_ALGO_OUTPUT
      algo->set_int("num_streamlines", num_seeds);
#endif
      }

    catch (const Exception &e)
    {
      algo_->error(std::string("Crashed with the following exception:\n") + e.message());
      success_[proc_num] = false;
    }
    catch (const std::string& a)
    {
      algo_->error(a);
      success_[proc_num] = false;
    }
    catch (const char *a)
    {
      algo_->error(a);
      success_[proc_num] = false;
    }

    return out;
  }

  void GenerateStreamLinesAccAlgo::find_nodes(std::vector<Point> &v, Point seed, bool back)
  {
    VMesh::Elem::index_type elem, neighbor;
    VMesh::Face::array_type faces;
    VMesh::Node::array_type nodes;
    VMesh::Face::index_type minface;
    VMesh::Face::index_type lastface;
    Vector lastnormal(0, 0, 0), minnormal(0, 0, 0);
    Vector dir;
    std::vector<Point> points(3);
    std::vector<Point> tv;

    if (!(mesh_->locate(elem, seed)))
    {
      return;
    }

    lastface = -1;

    tv.push_back(seed);

    for (int i = 0; i < max_steps_; i++)
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
      for (size_t j = 0; j < faces.size(); j++)
      {
        // Do not check last face as our see point is already on top of it
        if (faces[j] == lastface)
          continue;

        mesh_->get_nodes(nodes, faces[j]);
        mesh_->get_centers(points, nodes);
        Vector normal = Cross(points[1] - points[0], points[2] - points[0]);
        normal.safe_normalize();
        if (Dot(normal, ecenter - points[0]) > 0.0)
        {
          normal *= -1.0;
        }

        const double Vd = Dot(dir, normal);
        if (Vd < 1e-12) continue;
        const double V0 = Dot(normal, (points[0] - seed));
        const double dist = V0 / Vd;
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
      CleanupStreamLinePoints(tv, v, mesh_->get_epsilon()*mesh_->get_epsilon());
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

  if (method == IntegrationMethod::CellWalk && ifield->basis_order() != 0)
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
    mesh->synchronize(Mesh::EPSILON_E | Mesh::ELEM_LOCATE_E | Mesh::EDGES_E | Mesh::FACES_E);
  }
  else
  {
    mesh->synchronize(Mesh::EPSILON_E | Mesh::ELEM_LOCATE_E | Mesh::FACES_E);
  }

  bool success = false;

  if (method == IntegrationMethod::CellWalk)
  {
    detail::GenerateStreamLinesAccAlgo algo(this, IntegrationMethod::CellWalk);
    success = algo.run(input, seeds, output);
  }
  else
  {
    if (autoParams)
    {
      VMesh::size_type num_edges = mesh->num_edges();
      double length = 0;

      for (VMesh::Edge::index_type idx = 0; idx < num_edges; idx++)
      {
        length += mesh->get_size(idx);
      }
      length = length / num_edges;

#ifdef NEED_ADDITIONAL_OUTPUT_OBJECT
      set_scalar("tolerance", length / 20.0);
      set_scalar("step_size", length / 5.0);
#endif
    }

    detail::GenerateStreamLinesAlgoP algo(this, method);
    success = algo.run(input, seeds, output);
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
