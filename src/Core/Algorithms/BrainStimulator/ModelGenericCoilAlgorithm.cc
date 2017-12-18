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

#include <Core/Math/MiscMath.h>
#include <Core/Algorithms/BrainStimulator/ModelGenericCoilAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/BiotSavartSolverAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

#include <cassert>
#include <boost/lexical_cast.hpp>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(BrainStimulator, FigureOf8CoilShape);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Windings);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Current);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Radius);
ALGORITHM_PARAMETER_DEF(BrainStimulator, InnerRadius);
ALGORITHM_PARAMETER_DEF(BrainStimulator, OuterRadius);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Distance);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Layers);
ALGORITHM_PARAMETER_DEF(BrainStimulator, LayerStepSize);
ALGORITHM_PARAMETER_DEF(BrainStimulator, LevelOfDetail);
ALGORITHM_PARAMETER_DEF(BrainStimulator, OuterDistance);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Segments);

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
					pc(0)
				{
				}
				virtual ~BaseSegments()
				{
					this->Terminate();
				}

				//TODO consider splitting adding of value
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

					pc++;
				}

				virtual void Terminate()
				{
					pc = 0;
				}

				std::vector<Vector>& points;
				std::vector<size_t>& indices;
				std::vector<double>& values;
				size_t pc;

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
				virtual ~ClosedSegments()
				{
					if(pc)
					{
						this->Terminate();
					}

					//std::cout << "~ClosedSegments() Points Indices Values: " <<  points.size() << " " << indices.size() << " " << values.size() << std::endl;
					assert(points.size() > 0);
					assert(points.size() == values.size());
					assert(points.size()*2 == indices.size());
				}
				
				void Terminate()
				{
					size_t psize = points.size();

					//close the segments and make a circle
					indices.push_back(psize-1);
					indices.push_back(psize-pc);

					values.push_back(values[values.size()-1]);

					//std::cout << "ClosedSegment-Teminate-> psize:" << psize << " pc:" << pc << "    " << std::endl; 

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
				~OpenSegments()
				{
					assert(points.size() > 0);
					assert(indices.size() == values.size()*2);
					//assert(coilPoints.size()*2 - 2*coilLayers == coilIndices.size());
				}
		};



		class BaseCoilgen
		{
			public:
				BaseCoilgen(const AlgorithmBase* algo) :
				  ref_cnt(0),
				  coilLOD(3),
				  coilType(1),
				  coilLayers(1),
				  coilLayersStep(0),
				  algo(algo)
				{

				}
				
				virtual ~BaseCoilgen()
				{
				}
							
				//! Local entry function, must be implemented by each specific kernel
				virtual void Generate(FieldHandle& meshHandle) const = 0;
		
				//! Global reference counting
				int ref_cnt;
			
			protected:

				//! ref to the executing algorithm context
				const AlgorithmBase* algo;
				size_t coilLOD;
				size_t coilType;
				size_t coilLayers;
				double coilLayersStep;

				void GenPointsCircular(
					BaseSegments& segments,
					Vector origin, 
					double radius,
					double value,
					double fromPI,
					double toPI, 
					double extLOD = 0.0) const
				{

					double dPI = abs(toPI - fromPI);
					
					double minPI = M_PI /  ( 8.0 * coilLOD + coilLOD * extLOD );
					
					assert(dPI > minPI);
					
					size_t nsegments = 2;
					double iPI = dPI / nsegments;
					
					while(iPI > minPI)
					{
						nsegments++;
						iPI = dPI / nsegments;
					}

					algo->remark("#Segments(LOD):  " +  boost::lexical_cast<std::string>(nsegments) );
					
					dPI = toPI - fromPI;
					
					iPI = dPI / nsegments;

					for(size_t i = 0; i < nsegments; i++)
					{
						Vector point(origin.x() + radius * cos(fromPI + iPI*i), origin.y() + radius * sin(fromPI + iPI*i), origin.z());
						segments.AddPoint(point,value);
					}
				}

				// void GenPointsCircular2(
				// 	BaseSegments& segments,
				// 	Vector origin, 
				// 	double radius,
				// 	double nsegments,
				// 	double fromPI, 
				// 	double toPI) const
				// {										
				// 	double dPI = toPI - fromPI;
					
				// 	double iPI = dPI / nsegments;
					

				// 	for(size_t i = 0; i < nsegments; i++)
				// 	{
				// 		Vector p(origin.x() + radius * cos(fromPI + iPI*i), origin.y() + radius * sin(fromPI + iPI*i), origin.z());
				// 		segments.AddPoint(p);
				// 	}
				// }
				
				void BuildScirunMesh(
						const std::vector<Vector>& points, 
						const std::vector<size_t>& indices, 
						const std::vector<double>& values,
						FieldHandle& meshHandle) const
				{
					
					VMesh* mesh = meshHandle->vmesh();


					//! add nodes to the new mesh
					for(size_t i = 0; i < points.size(); i++)
					{
						const Point p(points[i]);
						mesh->add_point(p);
					}

					//! add edges to mesh
					VMesh::Node::array_type edge;

					for(size_t i = 0; i < indices.size(); i++)
					{
					  VMesh::Node::index_type p = indices[i];
					  edge.push_back(p);

					  if(edge.size() == 2)
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
		class SingleloopCoilgen : public BaseCoilgen
		{
			public:
			
				SingleloopCoilgen( 
					const AlgorithmBase* algo, 
					ModelTMSCoilSingleAlgorithm::Args args)
					: BaseCoilgen( algo ),
					  	radius(args.coilRadius),
					  	outerD(args.coilDistanceOuter),
					  	current(args.wireCurrent)
				{
					coilLOD = args.coilLevelDetails;
					coilType = args.type;
					coilLayers = args.coilLayers;

					coilLayers = coilLayers == 0 ? 1 : coilLayers;

					coilLayersStep = args.coilLayersStep;

					current = current / coilLayers;
				}
				
				~SingleloopCoilgen()
				{
				}
				
				virtual void Generate(FieldHandle& meshHandle) const
				{
					Vector vec;
					

					std::vector<Vector> coilPoints;
					std::vector<size_t> coilIndices;
					std::vector<double> coilValues;

					Vector step(0,0,coilLayersStep);
	  
					if(coilType == 1)
					{
						///SINGLE
						//Vector origin(0, 0, -0.5*(1.0/coilLayers));
						Vector origin(0, 0, -coilLayersStep*(coilLayers/2) );
						
						for(size_t l = 0; l < coilLayers; l++)
						{
							ClosedSegments segments(coilPoints,coilIndices,coilValues);

							GenPointsCircular(segments, origin, radius, current, 0.0, 2.0*M_PI);

							origin += step;
						}

						
					}
					else if(coilType == 2)
					{
						Vector originLeft( -radius - (outerD/2), 0, -coilLayersStep*(coilLayers/2));
						Vector originRight( radius + (outerD/2), 0, -coilLayersStep*(coilLayers/2));
						
						
						for(size_t l = 0; l < coilLayers; l++)
						{
							ClosedSegments segments(coilPoints,coilIndices,coilValues);

							///LEFT
							GenPointsCircular(segments, originLeft, radius, current, 0.0, 2.0*M_PI);
							
							originLeft += step;
						}

						for(size_t l = 0; l < coilLayers; l++)
						{
							ClosedSegments segments(coilPoints,coilIndices,coilValues);

							///RIGHT
							GenPointsCircular(segments, originRight, radius, -current, 0.0, 2.0*M_PI);
							
							originRight += step;
						}

					}
					else
					{
						algo->error("coil type value expeced: 1/2 (0-shape/8-shape)");
						return;
					}
					
					
					///SCIrun API creating a new mesh
					///0 data on elements; 1 data on nodes
					FieldInformation fi("CurveMesh",0,"double");
					fi.make_curvemesh();
					fi.make_constantdata();
					fi.make_scalar();

					meshHandle = CreateField(fi);

					BuildScirunMesh(coilPoints,coilIndices,coilValues,meshHandle);
				}
				
			protected:

				const double radius;
				double current;
				const double outerD;

		};
			
		//! piece-wise wire discretization
		class MultiloopsCoilgen : public BaseCoilgen
		{
			public:
			
				MultiloopsCoilgen( 
					const AlgorithmBase* algo, 
					ModelTMSCoilSpiralAlgorithm::Args args )
					: BaseCoilgen( algo ),
					  	innerR(args.coilRadiusInner),
					  	outerR(args.coilRadiusOuter),
					  	outerD(args.coilDistanceOuter),
					  	current(args.wireCurrent),
					  	windings(args.wireLoops)
				{
					coilLOD = args.coilLevelDetails;
					coilType = args.type;
					coilLayers = args.coilLayers;

					coilLayers = coilLayers == 0 ? 1 : coilLayers;

					coilLayersStep = args.coilLayersStep;

					//no auto current adjustment for each layer
					//hidden functionality (rather be explicit up front)
					//leave it up to users judgment

					//current = current / coilLayers;
				}
				
				~MultiloopsCoilgen()
				{
				}
				
				virtual void Generate(FieldHandle& meshHandle) const
				{
					std::vector<Vector> coilPoints;
					std::vector<size_t> coilIndices;
					std::vector<double> coilValues;

					Vector step(0,0,coilLayersStep);



					if(coilType == 1)
					{
						//Vector origin(0, 0, -0.5*(1.0/coilLayers));
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
					
					BuildScirunMesh(coilPoints,coilIndices,coilValues,meshHandle);
				}
				
			protected:

				const double innerR;
				const double outerR;
				double current;
				const double outerD;
				const size_t windings;
				
				void GenPointsSpiralLeft(OpenSegments& segments, Vector center) const

				{
					double dr = (outerR - innerR) / windings;		
					
					Vector center_offset (center.x() + dr/2, center.y(), center.z() );
					
					for (size_t i = 0; i < windings; i++)
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
					double dr = (outerR - innerR) / windings;		
					
					Vector center_offset( center.x() + dr/2, center.y(), center.z() );

					for (size_t i = windings; i > 0; i--)
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
				void AdjustForProximityEffect()
				{
					
				}
				
		};


		//! dipoles domain discretization
		// (replicating paper doi:10.1006/nimg.2002.1282)
		class DipolesCoilgen : public BaseCoilgen
		{
			public:
			
				DipolesCoilgen( 
					const AlgorithmBase* algo, 
					ModelTMSCoilDipoleAlgorithm::Args args )
					: BaseCoilgen( algo ),
					  	innerR(args.coilRadiusInner),
					  	outerR(args.coilRadiusOuter),
					  	outerD(args.coilDistanceOuter),
					  	current(args.totalCurrent),
					  	segments(args.numberSegments)
					  	
				{
					coilLOD = args.coilLevelDetails;
					coilType = args.type;
					coilLayers = args.coilLayers;

					coilLayers = coilLayers == 0 ? 1 : coilLayers;
				}
				
				~DipolesCoilgen()
				{
				}
				
				virtual void Generate(FieldHandle& meshHandle) const
				{
					std::vector<Vector> dipolePoints;
					std::vector<Vector> dipoleValues;
					std::vector<size_t> coilIndices;
					std::vector<double> radiiInner = preRadiiInner();
					std::vector<double> radiiOuter = preRadiiOuter();
					std::vector<double> numElements = preNumElem();
					std::vector<double> numCoupling = preNumAdjElem();

					
					if(coilType == 1)
					{
						Vector center(0, 0, 0);

						for (size_t i = 0; i < 16; i++)
						{
							double ringRad = radiiInner[i] + (radiiOuter[i] - radiiInner[i]) / 2.0;
							double ringArea = M_PI * ( radiiOuter[i] * radiiOuter[i] - radiiInner[i] * radiiInner[i] );
							double dipoleMoment = current * ringArea * numCoupling[i] / numElements[i];
							
							/// SINGLE COIL
							Vector dipoleNormL(0,0,1.0*dipoleMoment);
							GenPointsCircular2(dipolePoints, center, ringRad, 0.0, 2*M_PI, numElements[i]);
							GenSegmentValues(dipolePoints, dipoleValues, dipoleNormL );
						}
					}
					else if(coilType == 2)
					{
						Vector originL(- radiiOuter[15] - outerD / 2.0, 0, 0);
						Vector originR( radiiOuter[15] + outerD / 2.0, 0, 0 );
						
						for (size_t i = 0; i < 16; i++)
						{
							double ringRad = radiiInner[i] + (radiiOuter[i] - radiiInner[i]) / 2.0;
							double ringArea = M_PI * ( radiiOuter[i] * radiiOuter[i] - radiiInner[i] * radiiInner[i] );
							double dipoleMoment = current * ringArea * numCoupling[i] / numElements[i];
							
							
							/// LEFT COIL
							Vector dipoleNormL(0,0,1.0*dipoleMoment);
							GenPointsCircular2(dipolePoints, originL, ringRad, 0.0, 2*M_PI, numElements[i]);
							GenSegmentValues(dipolePoints, dipoleValues, dipoleNormL );


							/// RIGHT COIL
							Vector dipoleNormR(0,0,-1.0*dipoleMoment);
							GenPointsCircular2(dipolePoints, originR, ringRad, 0.0, 2*M_PI, numElements[i]);
							GenSegmentValues(dipolePoints, dipoleValues, dipoleNormR );
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
					
					BuildScirunMesh(dipolePoints,dipoleValues,meshHandle);
				}
				
		protected:

				const double innerR;
				const double outerR;
				const double current;
				const double outerD;
				const size_t segments;
				
				const std::vector<double> preRadiiInner() const
				{
					const double vals[16] = {0.00, 0.003, 0.007, 0.011, 0.015, 0.019, 0.023, 0.026, 0.028, 0.030, 0.032, 0.034, 0.036, 0.038, 0.040, 0.042};
					std::vector<double> preRadii(vals,vals+16);
					return preRadii;
				}
				
				const std::vector<double> preRadiiOuter() const
				{
					const double vals[16] = {0.003, 0.007, 0.011, 0.015, 0.019, 0.023, 0.026, 0.028, 0.030, 0.032, 0.034, 0.036, 0.038, 0.040, 0.042, 0.044};
					std::vector<double> preRadii(vals,vals+16);
					return preRadii;
				}

				const std::vector<double> preNumElem() const
				{
					const double vals[16] = {3.0, 9.0, 12.0, 16.0, 20.0, 24.0, 28.0, 30.0, 32.0, 34.0, 36.0, 38.0, 40.0, 42.0, 44.0, 44.0};
					std::vector<double> preNumElem(vals,vals+16);
					return preNumElem;

				}

				const std::vector<double> preNumAdjElem() const
				{
					const double vals[16] = {9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
					std::vector<double> preNumAdjElem(vals,vals+16);
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

				void GenPointsCircular2(
					std::vector<Vector>& points,
					Vector origin, 
					double radius,
					double fromPI,
					double toPI, 
					double extLOD) const
				{

					double dPI = abs(toPI - fromPI);
					
					double minPI = M_PI /  extLOD;
					
					size_t nsegments = (size_t)extLOD;
					double iPI = dPI / nsegments;
					

					algo->remark("#Segments(LOD):  " +  boost::lexical_cast<std::string>(nsegments) );
					
					dPI = toPI - fromPI;
					
					iPI = dPI / nsegments;

					for(size_t i = 0; i < nsegments; i++)
					{
						Vector point(origin.x() + radius * cos(fromPI + iPI*i), origin.y() + radius * sin(fromPI + iPI*i), origin.z());
						//segments.AddPoint(point,value);
						points.push_back(point);
					}
				}
				
				void BuildScirunMesh(const std::vector<Vector>& points, 
						const std::vector<Vector>& values,
						FieldHandle& meshHandle) const
				{
					
					VMesh* mesh = meshHandle->vmesh();

					//! add nodes to the new mesh
					for(size_t i = 0; i < points.size(); i++)
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


	bool ModelTMSCoilSingleAlgorithm::run(FieldHandle& meshFieldHandle, Args& args) const
	{
		
		try
		{
			std::vector<Vector> coilPoints;
			std::vector<size_t> coilIndices;
                         
			auto SC = new SingleloopCoilgen(this,args);
			SC->Generate(meshFieldHandle);

		}
		catch (...)
		{
			error("Error while running the algorithm ...");
			 return (false);
		}
               
		return (true);
	}

	bool ModelTMSCoilSpiralAlgorithm::run(FieldHandle& meshFieldHandle, Args& args) const
	{
		
		try
		{
			std::vector<Vector> coilPoints;
			std::vector<size_t> coilIndices;

			auto MC = new MultiloopsCoilgen(this,args);
			
			MC->Generate(meshFieldHandle);

		}
		catch (...)
		{
			error("Error while running the algorithm ...");
			return (false);
		}

		return (true);
	}


	bool ModelTMSCoilDipoleAlgorithm::run(FieldHandle& meshFieldHandle, Args& args) const
	{
		
		try
		{
			std::vector<Vector> coilPoints;
			std::vector<size_t> coilIndices;

                        auto DC = new DipolesCoilgen(this,args);
			DC->Generate(meshFieldHandle);

		}
		catch (...)
		{
			error("Error while running the algorithm ...");
			return (false);
		}

		return (true);
	}


AlgorithmOutput ModelTMSCoilSingleAlgorithm::run(const AlgorithmInput& input) const
{
  AlgorithmOutput output;
 
  ModelTMSCoilSingleAlgorithm::Args algoArgs;
  algoArgs.wireCurrent = static_cast<double>(get(Parameters::Current).toDouble())*1e6; ///MD: here is the unit scaling, wire current module input is in [megaA/s]
  algoArgs.coilRadius = static_cast<double>(get(Parameters::Radius).toDouble());
  algoArgs.coilDistanceOuter = static_cast<double>(get(Parameters::Distance).toDouble());
  algoArgs.coilLayers = static_cast<size_t>(get(Parameters::Layers).toInt());
  algoArgs.coilLevelDetails = static_cast<size_t>(get(Parameters::LevelOfDetail).toInt());
  algoArgs.coilLayersStep = static_cast<double>(get(Parameters::LayerStepSize).toDouble());
  bool coilshape = get(Parameters::FigureOf8CoilShape).toInt();
  if (coilshape)
    algoArgs.type = 2;
  else
    algoArgs.type = 1;
  
  FieldHandle ofield;
  
  if (!(run(ofield,algoArgs))) 
  {
    error(" ModelTMSCoilSingleAlgorithm failed. ");
  }
  
  output[Parameters::Mesh] = ofield;
  return output; 
}

AlgorithmOutput ModelTMSCoilSpiralAlgorithm::run(const AlgorithmInput& input) const
{
  AlgorithmOutput output;
 
  ModelTMSCoilSpiralAlgorithm::Args algoArgs;
  algoArgs.wireCurrent = static_cast<double>(get(Parameters::Current).toDouble())*1e6; ///MD: here is the unit scaling, wire current module input is in [megaA/s]
  algoArgs.wireLoops = static_cast<double>(get(Parameters::Windings).toInt());
  algoArgs.coilRadiusInner = static_cast<double>(get(Parameters::InnerRadius).toDouble());
  algoArgs.coilRadiusOuter = static_cast<double>(get(Parameters::OuterRadius).toDouble());
  algoArgs.coilLayers = static_cast<size_t>(get(Parameters::Layers).toInt());
  algoArgs.coilLevelDetails = static_cast<size_t>(get(Parameters::LevelOfDetail).toInt());
  algoArgs.coilLayersStep = static_cast<double>(get(Parameters::LayerStepSize).toDouble());
  algoArgs.coilDistanceOuter = static_cast<double>(get(Parameters::OuterDistance).toDouble());
  
  bool coilshape = get(Parameters::FigureOf8CoilShape).toBool();
  if (coilshape)
    algoArgs.type = 2;
  else
    algoArgs.type = 1;
  
  FieldHandle ofield;
  
  if (!(run(ofield,algoArgs))) 
  {
    error(" ModelTMSCoilSpiralAlgorithm failed. ");
  }
  
  output[Parameters::Mesh] = ofield;
  return output; 
}

AlgorithmOutput ModelTMSCoilDipoleAlgorithm::run(const AlgorithmInput& input) const
{
  AlgorithmOutput output;
	    
  ModelTMSCoilDipoleAlgorithm::Args algoArgs;
  algoArgs.totalCurrent = static_cast<double>(get(Parameters::Current).toDouble())*1e6;
  algoArgs.numberSegments = static_cast<size_t>(get(Parameters::Segments).toInt());
  algoArgs.coilRadiusInner = static_cast<double>(get(Parameters::InnerRadius).toDouble());
  algoArgs.coilRadiusOuter = static_cast<double>(get(Parameters::OuterRadius).toDouble());
  algoArgs.coilDistanceOuter = static_cast<double>(get(Parameters::OuterDistance).toDouble());
  algoArgs.coilLayers = static_cast<size_t>(get(Parameters::Layers).toInt());
  algoArgs.coilLevelDetails = static_cast<size_t>(get(Parameters::LevelOfDetail).toInt());
  
  bool coilshape = get(Parameters::FigureOf8CoilShape).toBool();
  if (coilshape)
    algoArgs.type = 2;
  else
    algoArgs.type = 1;
  
  FieldHandle ofield;
  
  if (!(run(ofield,algoArgs))) 
  {
    error(" ModelTMSCoilDipoleAlgorithm failed. ");
  }
  
  output[Parameters::Mesh] = ofield;
  return output; 
}

