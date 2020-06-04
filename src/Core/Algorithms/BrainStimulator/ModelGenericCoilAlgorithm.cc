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


#include <Core/Math/MiscMath.h>
#include <Core/Algorithms/BrainStimulator/ModelGenericCoilAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/BiotSavartSolverAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

#include <boost/lexical_cast.hpp>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(BrainStimulator, Type);
ALGORITHM_PARAMETER_DEF(BrainStimulator, FigureOf8CoilShape);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Rings);
ALGORITHM_PARAMETER_DEF(BrainStimulator, WingsAngle);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Current);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Radius);
ALGORITHM_PARAMETER_DEF(BrainStimulator, InnerRadius);
ALGORITHM_PARAMETER_DEF(BrainStimulator, OuterRadius);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Distance);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Layers);
ALGORITHM_PARAMETER_DEF(BrainStimulator, LayerStepSize);
ALGORITHM_PARAMETER_DEF(BrainStimulator, LevelOfDetail);


		class BaseSegments
		{
			public:
				BaseSegments(
					std::vector<Vector>& p,
					std::vector<size_t>& i,
					std::vector<double>& v):
					points(p),
					indices(i),
					values(v),
					pc(0),
					start_idx(0),
					end_idx(0)
				{
				}
				virtual ~BaseSegments()
				{
					this->Terminate();
				}

				void AddPoint(Vector point, double value)
				{
					points.push_back(point);

					size_t psize = points.size();

					if( pc > 0)
					{

						indices.push_back(psize-2);
						indices.push_back(psize-1);
						values.push_back(value);
					}
					else
					{
						start_idx = psize - 1;
					}

					++pc;
				}

				void Transform(Transform& t)
				{
					for(size_t i = start_idx; i < start_idx + pc; ++i)
					{
						points[i] = t * points[i];
					}
				}

				virtual void Terminate()
				{
					pc = 0;
				}

				std::vector<Vector>& points;
				std::vector<size_t>& indices;
				std::vector<double>& values;
				size_t pc;
				size_t start_idx;
				size_t end_idx;

			private:
				//! Prevent copying
    			BaseSegments & operator = (const BaseSegments & other);
    			BaseSegments(const BaseSegments & other);
		};

		class ClosedSegments : public BaseSegments
		{
			public:
				ClosedSegments(
					std::vector<Vector>& p,
					std::vector<size_t>& i,
					std::vector<double>& v)
						:BaseSegments(p,i,v)
				{
				}

				~ClosedSegments() override
				{
					if (pc)
					{
						this->Terminate();
					}
				}

				void Terminate() override
				{
					size_t psize = points.size();

					//close the segments and make a circle
					indices.push_back(psize - 1);
					indices.push_back(psize - pc);

					values.push_back(values[values.size()-1]);
					pc = 0;
				}
		};

		class OpenSegments : public BaseSegments
		{
			public:
				OpenSegments(
					std::vector<Vector>& p,
					std::vector<size_t>& i,
					std::vector<double>& v)
						:BaseSegments(p,i,v)
				{}
		};

		class BaseCoilgen
		{
			public:
				BaseCoilgen(const AlgorithmBase* algo, const ModelTMSCoilAlgorithm::Args& args) :
				  algo(algo),
				  coilLOD(args.coilLevelDetails),
				  coilType(args.type),
				  coilLayers(args.coilLayers),
				  rings(args.rings),
				  coilLayersStep(args.coilLayersStep),
				  wingsAngle(args.wingsAngle),
				  current(args.current),
				  innerR(args.coilRadiusInner),
				  outerR(args.coilRadiusOuter),
				  outerD(args.coilDistance)
				{
				}

				virtual ~BaseCoilgen()
				{
				}

				FieldHandle Execute() const
				{
          FieldHandle result;
					Generate(result);
          return result;
				}

			protected:
				//! ref to the executing algorithm context
				const AlgorithmBase* algo;
				size_t coilLOD;
				size_t coilType;
				size_t coilLayers;
				const size_t rings;
				double coilLayersStep;
				double wingsAngle;
				double current;
				const double innerR;
				const double outerR;
				const double outerD;

				//! Local entry function, must be implemented by each specific kernel
				virtual void Generate(FieldHandle& meshHandle) const = 0;

				void GenPointsCircular(
					BaseSegments& segments,
					Vector origin,
					double radius,
					double value,
					double fromPI,
					double toPI,
					double extLOD = 0.0) const
				{
					double dPI = fabs(toPI - fromPI);
					double minPI = M_PI /  ( 8.0 * coilLOD + coilLOD * extLOD );

					assert(dPI > minPI);

					size_t nsegments = 2;
					double iPI = dPI / nsegments;

					while (iPI > minPI)
					{
						nsegments++;
						iPI = dPI / nsegments;
					}

					//algo->remark("#Segments(LOD):  " +  boost::lexical_cast<std::string>(nsegments) );

					dPI = toPI - fromPI;
					iPI = dPI / nsegments;

					for (size_t i = 0; i < nsegments; i++)
					{
						Vector point(origin.x() + radius * cos(fromPI + iPI*i), origin.y() + radius * sin(fromPI + iPI*i), origin.z());
						segments.AddPoint(point,value);
					}
				}

				void BuildMesh(
						const std::vector<Vector>& points,
						const std::vector<size_t>& indices,
						const std::vector<double>& values,
						FieldHandle& meshHandle) const
				{
					VMesh* mesh = meshHandle->vmesh();

					//! add nodes to the new mesh
					for (size_t i = 0; i < points.size(); i++)
					{
						const Point p(points[i]);
						mesh->add_point(p);
					}

					//! add edges to mesh
					VMesh::Node::array_type edge;

					for (size_t i = 0; i < indices.size(); i++)
					{
					  VMesh::Node::index_type p = indices[i];
					  edge.push_back(p);

					  if (edge.size() == 2)
					  {
						  mesh->add_elem(edge);
						  edge.clear();
					  }
					}

					//! add data to mesh
					VField* field = meshHandle->vfield();
					field->resize_values();
					field->set_values(values);
				}
		};

		//! piece-wise wire discretization
		class CircularWireCoilgen : public BaseCoilgen
		{
			public:
				CircularWireCoilgen(const AlgorithmBase* algo, const ModelTMSCoilAlgorithm::Args& args )
					: BaseCoilgen( algo, args )
				{
					coilLayers = coilLayers == 0 ? 1 : coilLayers;
				}

				void Generate(FieldHandle& meshHandle) const override
				{
					std::vector<Vector> coilPoints;
					std::vector<size_t> coilIndices;
					std::vector<double> coilValues;

					Vector step(0,0,coilLayersStep);
					double dr = 0.0;
					if (rings > 1)
						dr = (outerR - innerR) / (rings - 1);

					if (coilType == 1)
					{
						Vector origin(0, 0, -coilLayersStep*(coilLayers/2) );

						for (size_t l = 0; l < coilLayers; l++)
						{
							for (size_t i = 0; i < rings; i++)
							{
								ClosedSegments segments(coilPoints,coilIndices,coilValues);
								GenPointsCircular(segments, origin, innerR + i*dr, current, 0.0 , 2*M_PI );
							}
							origin += step;
						}
					}
					else if (coilType == 2)
					{
						Vector originLeft ( -outerR - (outerD/2), 0.0, -coilLayersStep*(coilLayers/2) );
						Vector originRight(  outerR + (outerD/2), 0.0, -coilLayersStep*(coilLayers/2) );

						auto transLeft = Transform::Identity();
						transLeft.post_rotate(M_PI*(wingsAngle/180),{0,1,0});

						auto transRight = Transform::Identity();
						transRight.post_rotate(M_PI*(-wingsAngle/180),{0,1,0});

						for (size_t l = 0; l < coilLayers; l++)
						{
							for (size_t i = 0; i < rings; i++)
							{
								ClosedSegments segments(coilPoints,coilIndices,coilValues);
								GenPointsCircular(segments, originLeft, innerR + i*dr, current, 0.0 , 2*M_PI );
								segments.Transform(transLeft);
							}
							originLeft += step;
						}

						for (size_t l = coilLayers; l < 2*coilLayers; l++)
						{
							for (size_t i = 0; i < rings; i++)
							{
								ClosedSegments segments(coilPoints,coilIndices,coilValues);
								GenPointsCircular(segments, originRight, innerR + i*dr, -current, 0.0 , 2*M_PI);
								segments.Transform(transRight);
							}
							originRight += step;
						}
					}
					else
					{
						algo->error("coil type value expeced: 1/2 (0-shape/8-shape)");
						return;
					}

					//SCIrun API creating a new mesh
					//0 data on elements; 1 data on nodes
					FieldInformation fi("CurveMesh",0,"double");
					fi.make_curvemesh();
					fi.make_constantdata();
					fi.make_scalar();

					meshHandle = CreateField(fi);
					BuildMesh(coilPoints, coilIndices, coilValues, meshHandle);
				}
		};

		//! piece-wise wire discretization
		class MultiloopsCoilgen : public BaseCoilgen
		{
			public:
				MultiloopsCoilgen(const AlgorithmBase* algo, const ModelTMSCoilAlgorithm::Args& args )
					: BaseCoilgen( algo, args )
				{
					coilLayers = coilLayers == 0 ? 1 : coilLayers;

					//no auto current adjustment for each layer
					//hidden functionality (rather be explicit up front)
					//leave it up to users judgment
				}

				void Generate(FieldHandle& meshHandle) const override
				{
					std::vector<Vector> coilPoints;
					std::vector<size_t> coilIndices;
					std::vector<double> coilValues;

					Vector step(0,0,coilLayersStep);

					if(coilType == 1)
					{
						Vector origin(0, 0, -coilLayersStep*(coilLayers/2) );

						for(size_t l = 0; l < coilLayers; l++)
						{
							OpenSegments segments( coilPoints, coilIndices, coilValues );

							///SINGLE coil
							GenPointsSpiralLeft(segments, origin);
							origin += step;
						}

					}
					else if(coilType == 2)
					{
						Vector originLeft ( -outerR - (outerD/2), 0.0, -coilLayersStep*(coilLayers/2) );
						Vector originRight(  outerR + (outerD/2), 0.0, -coilLayersStep*(coilLayers/2) );

						for(size_t l = 0; l < coilLayers; l++)
						{
							OpenSegments segments( coilPoints, coilIndices, coilValues );

							//LEFT coil
							GenPointsSpiralLeft(segments, originLeft);
							originLeft += step;
						}

						for(size_t l = coilLayers; l < 2*coilLayers; l++)
						{
							OpenSegments segments( coilPoints, coilIndices, coilValues );

							//RIGHT coil
							GenPointsSpiralRight(segments, originRight);
							originRight += step;
						}
					}
					else
					{
						algo->error("coil type value expeced: 1/2 (0-shape/8-shape)");
						return;
					}

					//SCIrun API creating a new mesh
					//0 data on elements; 1 data on nodes
					FieldInformation fi("CurveMesh",0,"double");
					fi.make_curvemesh();
					fi.make_constantdata();
					fi.make_scalar();

					meshHandle = CreateField(fi);

					BuildMesh(coilPoints,coilIndices,coilValues,meshHandle);
				}

			protected:
				void GenPointsSpiralLeft(OpenSegments& segments, Vector center) const
				{
					double dr = (outerR - innerR) / rings;

					Vector center_offset (center.x() + dr/2, center.y(), center.z() );

					for (size_t i = 0; i < rings; i++)
					{
						GenPointsCircular(segments, center, innerR + i*dr, current, 0   , M_PI, i );
						GenPointsCircular(segments, center_offset, innerR + i*dr + dr/2, current, M_PI, 2*M_PI, i );
					}

					//TODO refactor to avoid this
					Vector endp(center.x() + outerR * cos(2*M_PI), center.y() + outerR * sin(2*M_PI), center.z());
					segments.AddPoint(endp, current);
				}

				void GenPointsSpiralRight(OpenSegments& segments, Vector center) const
				{
					double dr = (outerR - innerR) / rings;

					Vector center_offset( center.x() + dr/2, center.y(), center.z() );

					for (size_t i = rings; i > 0; i--)
					{
						GenPointsCircular(segments, center, innerR + i*dr, -current, M_PI, 2*M_PI, i );
						GenPointsCircular(segments, center_offset, innerR + i*dr - dr/2, -current, 0, M_PI, i );
					}

					//TODO refactor to avoid this
					Vector endp(center.x() + innerR * cos(M_PI), center.y() + innerR * sin(M_PI), center.z());
					segments.AddPoint(endp, -current);
				}

				/// this is tricky but doable, the idea is to distribute the current along each coil winding
				/// so that the top surface flux is not linear but curved (bell shaped like)
				/// this is required since in the AC profile there is inter-winding coupling increasing the resistivity of the net
				/// please see: https://en.wikipedia.org/wiki/Proximity_effect_%28electromagnetism%29
				// void AdjustForProximityEffect() {}
		};

		//! dipoles domain discretization
		// (replicating paper doi:10.1006/nimg.2002.1282)
		class DipolesCoilgen : public BaseCoilgen
		{
			public:
				DipolesCoilgen(const AlgorithmBase* algo, const ModelTMSCoilAlgorithm::Args& args )
					: BaseCoilgen( algo, args )
				{
					coilLayers = coilLayers == 0 ? 1 : coilLayers;
				}

				void Generate(FieldHandle& meshHandle) const override
				{
					std::vector<Vector> dipolePoints;
					std::vector<Vector> dipoleValues;

					Vector step(0,0,coilLayersStep);

					double dr = 0.0;
					if(rings > 1)
						dr = (outerR - innerR) / (rings - 1);

					if(coilType == 1)
					{
						Vector origin(0, 0, -coilLayersStep*(coilLayers/2) );

						for(size_t l = 0; l < coilLayers; l++)
						{
							double dipoleMoment = 0;
							for (size_t i = 0; i < rings; i++)
							{
								double ringArea = M_PI * (innerR + i*dr) * (innerR + i*dr);
								dipoleMoment += current * ringArea;
							}
							Vector dipoleNormL(0,0,1.0*dipoleMoment);
							dipolePoints.push_back(origin);
							dipoleValues.push_back(dipoleNormL);

							origin += step;
						}
					}
					else if(coilType == 2)
					{
						Vector originLeft ( -outerR - (outerD/2), 0.0, -coilLayersStep*(coilLayers/2) );
						Vector originRight(  outerR + (outerD/2), 0.0, -coilLayersStep*(coilLayers/2) );

						auto transLeft = Transform::Identity();
						transLeft.post_rotate(M_PI*(wingsAngle/180),{0,1,0});

						auto transRight = Transform::Identity();
						transRight.post_rotate(M_PI*(-wingsAngle/180),{0,1,0});

						for(size_t l = 0; l < coilLayers; l++)
						{
							double dipoleMoment = 0;
							for (size_t i = 0; i < rings; i++)
							{
								double ringArea = M_PI * (innerR + i*dr) * (innerR + i*dr);
								dipoleMoment += current * ringArea;
							}
							Vector dipoleNormL(0,0,1.0*dipoleMoment);
							dipoleNormL = transLeft * dipoleNormL;
							Vector originLeftTrans = transLeft * originLeft;
							dipolePoints.push_back(originLeftTrans);
							dipoleValues.push_back(dipoleNormL);

							originLeft += step;
						}

						for(size_t l = 0; l < coilLayers; l++)
						{
							double dipoleMoment = 0;
							for (size_t i = 0; i < rings; i++)
							{
								double ringArea = M_PI * (innerR + i*dr) * (innerR + i*dr);
								dipoleMoment += current * ringArea;
							}
							Vector dipoleNormR(0,0,-1.0*dipoleMoment);
							dipoleNormR = transRight * dipoleNormR;
							Vector originRightTrans = transRight * originRight;
							dipolePoints.push_back(originRightTrans);
							dipoleValues.push_back(dipoleNormR);

							originRight += step;
						}
					}
					else
					{
						algo->error("coil type value expeced: 1/2 (0-shape/8-shape)");
						return;
					}

					///basic topoly assumptions needs to be correct
					assert(dipolePoints.size() > 0);
					assert(dipolePoints.size() == dipoleValues.size());

					///SCIrun API creating a new mesh
					///0 data on elements; 1 data on nodes
					FieldInformation fi("PointCloudMesh",1,"vector");
					fi.make_pointcloudmesh();
					fi.make_lineardata();
					fi.make_vector();

					meshHandle = CreateField(fi);

					BuildMesh(dipolePoints, dipoleValues, meshHandle);
				}

		protected:
			const std::vector<double> preRadiiInner() const
			{
				std::vector<double> preRadii;
				double step = (outerR - innerR) / rings;
				double d = innerR;

				while (d < outerR)
				{
					preRadii.push_back(d);
					d += step;
				}
				return preRadii;
			}

			const std::vector<double> preRadiiOuter() const
			{
				std::vector<double> preRadii;
				double step = (outerR - innerR) / rings;
				double d = innerR;

				while (d < outerR)
				{
					d += step;
					preRadii.push_back(d);
				}

				//override last to fill to exacct outer radius
				preRadii[preRadii.size()-1u] = outerR;
				return preRadii;
			}

			const std::vector<double> preNumElem(const std::vector<double>& radii) const
			{
				std::vector<double> preNumElem;
				for(size_t i = 1; i <= radii.size(); ++i)
				{
					size_t n = M_PI_2 / (1.0 / ( radii[i]* coilLOD ) );
					preNumElem.push_back(n);
				}
				return preNumElem;
			}

			const std::vector<double> preNumAdjElem(const std::vector<double>& radii) const
			{
				std::vector<double> preNumAdjElem;
				for(size_t i = 1; i <= radii.size(); ++i)
				{
					preNumAdjElem.push_back(1.0);
				}
				return preNumAdjElem;
			}

			void GenSegmentValues(const std::vector<Vector>& points, std::vector<Vector>& values, Vector val) const
			{
				assert(points.size() > 0);

				for(size_t i = values.size(); i < points.size(); i++)
				{
					values.push_back(val);
				}
			}

				size_t GenPointsCircular2(
					std::vector<Vector>& points,
					Vector origin,
					double radius,
					double fromPI,
					double toPI,
					double extLOD) const
				{
					double dPI = fabs(toPI - fromPI);
					double minPI = M_PI /  ( 8.0 * coilLOD + coilLOD * extLOD );

					assert(dPI > minPI);

					size_t nsegments = 2;
					double iPI = dPI / nsegments;

					while (iPI > minPI)
					{
						nsegments++;
						iPI = dPI / nsegments;
					}

					//algo->remark("#Segments(LOD):  " +  boost::lexical_cast<std::string>(nsegments) );

					dPI = toPI - fromPI;
					iPI = dPI / nsegments;

					for(size_t i = 0; i < nsegments; i++)
					{
						Vector point(origin.x() + radius * cos(fromPI + iPI*i), origin.y() + radius * sin(fromPI + iPI*i), origin.z());
						points.push_back(point);
					}

					return nsegments;
				}

				void BuildMesh(const std::vector<Vector>& points,
						const std::vector<Vector>& values,
						FieldHandle& meshHandle) const
				{
					VMesh* mesh = meshHandle->vmesh();

					//! add nodes to the new mesh
					for (size_t i = 0; i < points.size(); i++)
					{
						const Point p(points[i]);
						mesh->add_point(p);
					}

					//! add data to mesh
					VField* field = meshHandle->vfield();
					field->resize_values();
					field->set_values(values);
				}
		};


int ResolveAlgoName(const std::string& name)
{
	if (name == "Thin Circular Wires")
	{
		return 0;
	}
	if (name == "Thin Spiral Wires")
	{
		return 1;
	}
	if (name == "Magnetic Dipoles")
	{
		return 2;
	}
	return 0;
}

std::unique_ptr<BaseCoilgen> AlgoSelector(int idx,const AlgorithmBase* scirunAlgoBase, const ModelTMSCoilAlgorithm::Args& args)
{
	switch(idx)
	{
		case 0: return std::unique_ptr<BaseCoilgen>(new CircularWireCoilgen(scirunAlgoBase,args));
		case 1: return std::unique_ptr<BaseCoilgen>(new MultiloopsCoilgen(scirunAlgoBase,args));
		case 2: return std::unique_ptr<BaseCoilgen>(new DipolesCoilgen(scirunAlgoBase,args));
		default: return std::unique_ptr<BaseCoilgen>(new CircularWireCoilgen(scirunAlgoBase,args));
	}
}


AlgorithmOutput ModelTMSCoilAlgorithm::run(const AlgorithmInput& input) const
{
	ModelTMSCoilAlgorithm::Args algoArgs;
	algoArgs.current = static_cast<double>(get(Parameters::Current).toDouble())*1e6; ///MD: here is the unit scaling, wire current module input is in [megaA/s]
	algoArgs.rings = static_cast<size_t>(get(Parameters::Rings).toInt());
	algoArgs.wingsAngle = static_cast<double>(get(Parameters::WingsAngle).toDouble());
	algoArgs.coilRadiusInner = static_cast<double>(get(Parameters::InnerRadius).toDouble());
	algoArgs.coilRadiusOuter = static_cast<double>(get(Parameters::OuterRadius).toDouble());
	algoArgs.coilLayers = static_cast<size_t>(get(Parameters::Layers).toInt());
	algoArgs.coilLevelDetails = static_cast<size_t>(get(Parameters::LevelOfDetail).toInt());
	algoArgs.coilLayersStep = static_cast<double>(get(Parameters::LayerStepSize).toDouble());
	algoArgs.coilDistance = static_cast<double>(get(Parameters::Distance).toDouble());
	algoArgs.type = get(Parameters::FigureOf8CoilShape).toInt() ? 2 : 1;

	auto algoImpl = AlgoSelector(ResolveAlgoName(get(Parameters::Type).toString()),this,algoArgs);

  AlgorithmOutput output;
	output[Parameters::Mesh] = algoImpl->Execute();
	return output;
}
