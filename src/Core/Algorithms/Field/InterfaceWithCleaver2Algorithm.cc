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


#include <Core/Algorithms/Field/InterfaceWithCleaver2Algorithm.h>
#include <Core/Algorithms/Field/InterfaceWithCleaverAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

#include <cleaver2/vec3.h>
#include <cleaver2/BoundingBox.h>
#include <cleaver2/Cleaver.h>
#include <cleaver2/CleaverMesher.h>
#include <cleaver2/InverseField.h>
#include <cleaver2/SizingFieldCreator.h>
#include <cleaver2/Volume.h>
#include <cleaver2/TetMesh.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <iostream>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Utils/StringUtil.h>
#include <boost/scoped_ptr.hpp>
#include <Core/Logging/Log.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;

ALGORITHM_PARAMETER_DEF(Fields, Verbose);
ALGORITHM_PARAMETER_DEF(Fields, VolumeScaling);
ALGORITHM_PARAMETER_DEF(Fields, VolumeMultiplier);
ALGORITHM_PARAMETER_DEF(Fields, Lipschitz);
ALGORITHM_PARAMETER_DEF(Fields, Padding);
ALGORITHM_PARAMETER_DEF(Fields, AlphaShort);
ALGORITHM_PARAMETER_DEF(Fields, AlphaLong);
ALGORITHM_PARAMETER_DEF(Fields, SimpleMode);
ALGORITHM_PARAMETER_DEF(Fields, MeshMode);
ALGORITHM_PARAMETER_DEF(Fields, ReverseJacobians);

const AlgorithmInputName InterfaceWithCleaver2Algorithm::SizingField("SizingField");
const AlgorithmInputName InterfaceWithCleaver2Algorithm::BackgroundField("BackgroundField");
const AlgorithmOutputName InterfaceWithCleaver2Algorithm::SizingFieldUsed("SizingFieldUsed");
const AlgorithmOutputName InterfaceWithCleaver2Algorithm::BackgroundFieldUsed("BackgroundFieldUsed");

namespace detail
{
  using CleaverScalarField = boost::shared_ptr<cleaver2::ScalarField<float>>;
  using CleaverInputField = boost::shared_ptr<cleaver2::AbstractScalarField>;
  using CleaverInputFieldList = std::vector<CleaverInputField>;

  static const double kDefaultAlpha = 0.4;
  static const double kDefaultAlphaLong = 0.357;
  static const double kDefaultAlphaShort = 0.203;
  static const double kDefaultSamplingRate = 1.0;
  static const double kDefaultLipschitz = 0.2;
  static const double kDefaultFeatureScaling = 1.0;
  //static const int    kDefaultPadding = 0;
  //static const int    kDefaultMaxIterations = 1000;
  //static const double kDefaultSigma = 1.;

  struct Cleaver2Parameters
  {
    cleaver2::MeshType mesh_mode;
    double alphaLong;
    double alphaShort;
    double samplingRate;
    double lipschitz;
    double featureScaling;
    bool verbose;
    bool simpleMode;
    bool reverseJacobians;
  };

  int dataSize(CleaverScalarField field)
  {
    auto databounds = field->dataBounds();
    int w = (int)databounds.size.x;
    int h = (int)databounds.size.y;
    int d = (int)databounds.size.z;
    return w * h * d;
  }

  class Cleaver2Impl
  {
  public:
    Cleaver2Impl(const AlgorithmBase* algo, const Cleaver2Parameters& params, FieldHandle sizingField, FieldHandle backgroundMesh) :
      algo_(algo), params_(params), inputSizingField_(sizingField), inputBackgroundMesh_(backgroundMesh)
    {
      LOG_DEBUG("Cleaver 2 parameters: \n\tmesh_mode: {}\n\talphaLong: {}\n\talphaShort: {}\n\tsampling_rate: {}\n\tlipschitz: {}\n\tfeature_scaling: {}\n\tverbose: {}\n\tsimpleMode: {}",
        params_.mesh_mode, params_.alphaLong, params_.alphaShort,
        params_.samplingRate, params_.lipschitz, params_.featureScaling,
        params_.verbose, params_.simpleMode);
    }

    FieldHandle cleave(CleaverInputFieldList fields)
    {
      volume_.reset(new cleaver2::Volume(toVectorOfRawPointers(fields)));

      // TODO DAN: add optional padding to sizing field...
      /// Padding is now optional!
      //boost::shared_ptr<cleaver2::AbstractVolume> paddedVolume(volume);
      // const bool verbose = get(Verbose).toBool();
      // const bool pad = get(Padding).toBool();
      //
      // if (pad)
      // {
      //   paddedVolume.reset(new Cleaver::PaddedVolume(volume.get()));
      // }
      //
      // if (verbose)
      // {
      //  std::cout << "Input Dimensions: " << dims[0] << " x " << dims[1] << " x " << dims[2] << std::endl;
      //  if (pad)
      //   std::cout << "Padded Mesh with Volume Size " << paddedVolume->size().toString() << std::endl;
      //      else
      //         std::cout << "Creating Mesh with Volume Size " << volume->size().toString() << std::endl;
      // }

      addSizingFieldToVolume();

      bool verbose = params_.verbose;

      bool simple = params_.simpleMode;
      cleaver2::CleaverMesher mesher(simple);
      mesher.setVolume(volume_.get());
      const double alpha = kDefaultAlpha; // do not expose
      mesher.setAlphaInit(alpha);

      //-----------------------------------
      // Load background mesh if provided
      //-----------------------------------
      cleaver2::TetMesh* bgMesh = nullptr;
      if (inputBackgroundMesh_)
      {
        bgMesh = makeCleaver2TetVolFromSCIRunTetVol(inputBackgroundMesh_);
        mesher.setBackgroundMesh(bgMesh);
        outputBackgroundMesh_ = inputBackgroundMesh_;
      }
      else
      {
        switch (params_.mesh_mode)
        {
          case cleaver2::Regular:
          {
            double alpha_long = kDefaultAlphaLong;
            double alpha_short = kDefaultAlphaShort;
            mesher.setAlphas(alpha_long, alpha_short);
            mesher.setRegular(true);
            bgMesh = mesher.createBackgroundMesh(verbose);
            break;
          }
          case cleaver2::Structured:
          {
            mesher.setRegular(false);
            bgMesh = mesher.createBackgroundMesh(verbose);
            break;
          }
        }
        outputBackgroundMesh_ = convertCleaverOutputToField(bgMesh);
      }

      mesher.buildAdjacency(verbose);
      mesher.sampleVolume(verbose);
      mesher.computeAlphas(verbose);
      mesher.computeInterfaces(verbose);
      mesher.generalizeTets(verbose);
      mesher.snapsAndWarp(verbose);
      mesher.stencilTets(verbose);

      auto mesh(boost::shared_ptr<cleaver2::TetMesh>(mesher.getTetMesh()));

      if (params_.reverseJacobians)
        mesh->fixVertexWindup(verbose);

      return convertCleaverOutputToField(mesh.get());
    }

    FieldHandle run(FieldList inputs)
    {
      detail::CleaverInputFieldList fields;

      for (auto& input : inputs)
      {
        fields.push_back(convertToCleaverFormat(input));
      }
      return cleave(fields);
    }

    void resetLatVolConverter()
    {
      x_ = y_ = z_ = 0;
    }

    CleaverScalarField convertToCleaverFormat(FieldHandle input)
    {
      VMesh::dimension_type dims;
      auto imesh1 = input->vmesh();

      if (!imesh1->is_structuredmesh())
      {
        THROW_ALGORITHM_INPUT_ERROR_WITH(algo_, "needs to be structured mesh!");
      }
      else
      {
        auto vfield1 = input->vfield();
        if (!vfield1->is_scalar())
        {
          THROW_ALGORITHM_INPUT_ERROR_WITH(algo_, "values at the node needs to be scalar!");
        }

        imesh1->get_dimensions(dims);
        if (x_ == 0)
        {
          x_ = dims[0]; y_ = dims[1]; z_ = dims[2];
          if (x_ < 1 || y_ < 1 || z_ < 1)
          {
            THROW_ALGORITHM_INPUT_ERROR_WITH(algo_, " Size of input fields should be non-zero !");
          }
        }
        else
        {
          if (dims[0] != x_ || dims[1] != y_ || dims[2] != z_)
          {
            THROW_ALGORITHM_INPUT_ERROR_WITH(algo_, " Size of input fields is inconsistent !");
          }
        }

        if (dims.size() != 3)
        {
          THROW_ALGORITHM_INPUT_ERROR_WITH(algo_, "need a three dimensional indicator function");
        }

        //0 = constant, 1 = linear
        if (1 != vfield1->basis_order())
        {
          THROW_ALGORITHM_INPUT_ERROR_WITH(algo_, "Input data need to be defined on input mesh nodes.");
        }

        if (vfield1->is_float())
        {
          auto ptr = static_cast<float*>(vfield1->fdata_pointer());
          if (ptr)
          {
            return makeCleaver2FieldFromLatVol(input);
          }
          else
          {
            THROW_ALGORITHM_INPUT_ERROR_WITH(algo_, "float field is NULL pointer");
          }
        }
        else
        {
          THROW_ALGORITHM_INPUT_ERROR_WITH(algo_, "Input field needs to be a structured mesh (best would be a LatVol) with float values defined on mesh nodes. ");
        }
      }
    }

    //TODO dan: need run-time check for float or double field data
    static CleaverScalarField makeCleaver2FieldFromLatVol(FieldHandle field)
    {
      auto vmesh = field->vmesh();
      auto vfield = field->vfield();
      VMesh::dimension_type dims;
      vmesh->get_dimensions(dims);

      auto ptr = static_cast<float*>(vfield->fdata_pointer());

      auto cleaverField = boost::make_shared<cleaver2::ScalarField<float>>(ptr, dims[0], dims[1], dims[2]);
      cleaver2::BoundingBox bb(cleaver2::vec3::zero, cleaver2::vec3(dims[0], dims[1], dims[2]));
      cleaverField->setBounds(bb);
      const auto& transform = vmesh->get_transform();

      int x_spacing = fabs(transform.get_mat_val(0, 0)), y_spacing = fabs(transform.get_mat_val(1, 1)), z_spacing = fabs(transform.get_mat_val(2, 2));

      if (IsNan(x_spacing) || x_spacing <= 0) x_spacing = 1; /// dont allow negative or zero scaling of the bounding box
      if (IsNan(y_spacing) || y_spacing <= 0) y_spacing = 1;
      if (IsNan(z_spacing) || z_spacing <= 0) z_spacing = 1;

      cleaverField->setScale(cleaver2::vec3(x_spacing, y_spacing, z_spacing));

      return cleaverField;
    }

    static cleaver2::TetMesh* makeCleaver2TetVolFromSCIRunTetVol(FieldHandle field)
    {
      FieldInformation info(field);
      if (!info.is_tetvol())
        return nullptr;

      auto vmesh = field->vmesh();
      const int numVertices = vmesh->num_nodes();

      double xmin = 1.0e16, xmax = -1.0e16,
                 ymin = 1.0e16, ymax = -1.0e16,
                 zmin = 1.0e16, zmax = -1.0e16;

      using namespace cleaver2;
      std::vector<Vertex*> verts(numVertices);

      for(VMesh::Node::index_type i = 0; i < numVertices; ++i)
      {
        auto point = vmesh->get_point(i);
        auto x = static_cast<float>(point.x());
        auto y = static_cast<float>(point.y());
        auto z = static_cast<float>(point.z());

        if (x < xmin) xmin = x;
        if (x > xmax) xmax = x;

        if (y < ymin) ymin = y;
        if (y > ymax) ymax = y;

        if (z < zmin) zmin = z;
        if (z > zmax) zmax = z;

        verts[i] = new Vertex();
        verts[i]->pos() = vec3(x,y,z);
        verts[i]->tm_v_index = i;
      }

      std::unique_ptr<TetMesh> mesh(new TetMesh(verts, {}));

      mesh->bounds = BoundingBox(xmin, ymin, zmin, xmax - xmin, ymax - ymin, zmax - zmin);

      int numElements = vmesh->num_elems();

      for (VMesh::Elem::index_type i = 0; i < numElements; ++i)
      {
        VMesh::Elem::array_type tetidx;
        vmesh->get_elems( tetidx, i );
        mesh->createTet(
            verts[tetidx[0]],
            verts[tetidx[1]],
            verts[tetidx[2]],
            verts[tetidx[3]],
            0);
      }

      return mesh.release();
    }

    static Point toPoint(const cleaver2::vec3& v)
    {
      return Point(v.x, v.y, v.z);
    }

    static FieldHandle makeLatVolFromCleaver2Field(CleaverScalarField cfield)
    {
      FieldInformation lfi(LATVOLMESH_E, LINEARDATA_E, FLOAT_E);

      auto cbbox = cfield->bounds();
      auto cdatabbox = cfield->dataBounds();

      logInfo("Cleaver sizing field: CenteringType {}, bounds {},{}; dataBounds {},{}", cfield->getCenterType(),
        cbbox.origin.toString(), cbbox.size.toString(),
        cdatabbox.origin.toString(), cdatabbox.size.toString()
      );

      auto mesh = CreateMesh(lfi, cdatabbox.size.x, cdatabbox.size.y, cdatabbox.size.z, toPoint(cbbox.minCorner()), toPoint(cbbox.maxCorner()));
      auto field = CreateField(lfi, mesh);

      auto vfield = field->vfield();

      vfield->resize_values();
      vfield->set_values(cfield->data(), dataSize(cfield));

      // const auto& transform = vmesh->get_transform();
      //
      // int x_spacing = fabs(transform.get_mat_val(0, 0)), y_spacing = fabs(transform.get_mat_val(1, 1)), z_spacing = fabs(transform.get_mat_val(2, 2));
      //
      // if (IsNan(x_spacing) || x_spacing <= 0) x_spacing = 1; /// dont allow negative or zero scaling of the bounding box
      // if (IsNan(y_spacing) || y_spacing <= 0) y_spacing = 1;
      // if (IsNan(z_spacing) || z_spacing <= 0) z_spacing = 1;
      //
      // cleaverField->setScale(cleaver2::vec3(x_spacing, y_spacing, z_spacing));

      return field;
    }

    void addSizingFieldToVolume()
    {
      if (inputSizingField_)
      {
        double min;
        inputSizingField_->vfield()->min(min);
        if (min <= 0)
          THROW_ALGORITHM_INPUT_ERROR_WITH(algo_, "Sizing field must contain only positive values.");

        //TODO: sloppy--need a const version
        resetLatVolConverter();
        sizingField_ = convertToCleaverFormat(inputSizingField_);
        outputSizingField_ = inputSizingField_;
      }
      else
      {
        sizingField_.reset(cleaver2::SizingFieldCreator::createSizingFieldFromVolume(
          volume_.get(),
          (float)(1.0 / params_.lipschitz),
          (float)params_.samplingRate,
          (float)params_.featureScaling,
          0, // padding--off
          (params_.mesh_mode != cleaver2::Regular),
          params_.verbose));
        outputSizingField_ = makeLatVolFromCleaver2Field(sizingField_);
      }
      volume_->setSizingField(sizingField_.get());
    }

    FieldHandle convertCleaverOutputToField(cleaver2::TetMesh* mesh)
    {
      auto nr_of_tets = mesh->tets.size();
      auto nr_of_verts = mesh->verts.size();

      if (nr_of_tets == 0 || nr_of_verts == 0)
      {
        THROW_ALGORITHM_INPUT_ERROR_WITH(algo_, " Number of resulting tetrahedral nodes or elements is 0. If you disabled padding enable it and execute again. ");
      }

      FieldInformation fi("TetVolMesh", 0, "double");   ///create output field

      auto output = CreateField(fi);
      auto omesh = output->vmesh();
      auto ofield = output->vfield();

      omesh->node_reserve(nr_of_verts);
      omesh->elem_reserve(nr_of_tets);

      for (auto i = 0; i < nr_of_verts; i++)
      {
        omesh->add_point(Point(mesh->verts[i]->pos().x, mesh->verts[i]->pos().y, mesh->verts[i]->pos().z));
      }

      VMesh::Node::array_type vdata;
      vdata.resize(4);
      std::vector<double> values(nr_of_tets);

      for (auto i = 0; i < nr_of_tets; i++)
      {
        vdata[0] = mesh->tets[i]->verts[0]->tm_v_index;
        vdata[1] = mesh->tets[i]->verts[1]->tm_v_index;
        vdata[2] = mesh->tets[i]->verts[2]->tm_v_index;
        vdata[3] = mesh->tets[i]->verts[3]->tm_v_index;
        omesh->add_elem(vdata);
        auto mat_label = mesh->tets[i]->mat_label;
        values[i] = mat_label;
      }
      ofield->resize_values();
      ofield->set_values(values);
      mesh->computeAngles();

      std::ostringstream ostr1, ostr2;
      ostr1 << "(nodes, elements, dims) - (" << nr_of_verts << " , " << nr_of_tets << " , " << volume_->size().toString() << ")" << std::endl;
      ostr2 << "(min angle, max angle) - (" << mesh->min_angle << " , " << mesh->max_angle << ")" << std::endl;

      algo_->remark(ostr1.str());
      algo_->remark(ostr2.str());
      return output;
    }

    FieldHandle sizingFieldUsed() const { return outputSizingField_; }
    FieldHandle backgroundMeshUsed() const { return outputBackgroundMesh_; }
  private:
    const AlgorithmBase* algo_;
    Cleaver2Parameters params_;
    FieldHandle inputSizingField_, outputSizingField_, inputBackgroundMesh_, outputBackgroundMesh_;
    CleaverScalarField sizingField_;
    boost::shared_ptr<cleaver2::TetMesh> backgroundMesh_;
    boost::shared_ptr<cleaver2::Volume> volume_;
    int x_ = 0, y_ = 0, z_ = 0;
  };
}

InterfaceWithCleaver2Algorithm::InterfaceWithCleaver2Algorithm()
{
  //Cleaver parameters: Scaling = Sampling Rate, Multiplier = Feature Scaling
  addParameter(Parameters::Verbose, true);
  addParameter(Parameters::SimpleMode, false);
  addParameter(Parameters::ReverseJacobians, true);
  addParameter(Parameters::VolumeScaling, detail::kDefaultSamplingRate);
  addParameter(Parameters::VolumeMultiplier, detail::kDefaultFeatureScaling);
  addParameter(Parameters::Lipschitz, detail::kDefaultLipschitz);
  //addParameter(Parameters::Padding, detail::kDefaultPadding);
  addParameter(Parameters::AlphaLong, detail::kDefaultAlphaLong);
  addParameter(Parameters::AlphaShort, detail::kDefaultAlphaShort);
  addParameter(Parameters::MeshMode, static_cast<int>(cleaver2::MeshType::Regular));
}

AlgorithmOutput InterfaceWithCleaver2Algorithm::runImpl(const FieldList& input, FieldHandle backgroundMesh, FieldHandle sizingField) const
{
  FieldList inputs;
  std::copy_if(input.begin(), input.end(), std::back_inserter(inputs), [](FieldHandle f) { return f; });

  if (inputs.empty())
  {
    THROW_ALGORITHM_INPUT_ERROR("No input fields given");
  }
  if (inputs.size() < 2)
  {
    THROW_ALGORITHM_INPUT_ERROR("At least 2 indicator functions stored as float values are needed to run cleaver!");
  }

  std::ostringstream ostr0;
  ostr0 << "Be aware that inside and outside of materials (to be meshed) need to be defined as positive and negative (e.g. surface distance) values across all module inputs. The zero crossings represents material boundaries." << std::endl;
  remark(ostr0.str());

  detail::Cleaver2Parameters params
  {
    cleaver2::MeshType(get(Parameters::MeshMode).toInt()),
    get(Parameters::AlphaLong).toDouble(),
    get(Parameters::AlphaShort).toDouble(),
    get(Parameters::VolumeScaling).toDouble(),
    get(Parameters::Lipschitz).toDouble(),
    get(Parameters::VolumeMultiplier).toDouble(),
    get(Parameters::Verbose).toBool(),
    get(Parameters::SimpleMode).toBool(),
    get(Parameters::ReverseJacobians).toBool()
  };
  detail::Cleaver2Impl impl(this, params, sizingField, backgroundMesh);
  auto tetmesh = impl.run(inputs);
  auto sizing = impl.sizingFieldUsed();
  auto background = impl.backgroundMeshUsed();

  AlgorithmOutput output;
  output[Variables::OutputField] = tetmesh;
  output[SizingFieldUsed] = sizing;
  output[BackgroundFieldUsed] = background;
  return output;
}

AlgorithmOutput InterfaceWithCleaver2Algorithm::run(const AlgorithmInput& input) const
{
  auto inputfields = input.getList<Field>(Variables::InputFields);
  auto sizingField = input.get<Field>(SizingField);

  return runImpl(inputfields, nullptr, sizingField);
}
