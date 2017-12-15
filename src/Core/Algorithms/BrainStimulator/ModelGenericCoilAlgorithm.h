/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef CORE_ALGORITHMS_BRAINSTIMULATOR_MODELGENERICCOIL_H
#define CORE_ALGORITHMS_BRAINSTIMULATOR_MODELGENERICCOIL_H 1

#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Matrix.h>

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/BrainStimulator/share.h>

#include <string>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace BrainStimulator {

ALGORITHM_PARAMETER_DECL(Type);
ALGORITHM_PARAMETER_DECL(FigureOf8CoilShape);
ALGORITHM_PARAMETER_DECL(Rings);
ALGORITHM_PARAMETER_DECL(Current);
ALGORITHM_PARAMETER_DECL(Radius);
ALGORITHM_PARAMETER_DECL(InnerRadius);
ALGORITHM_PARAMETER_DECL(OuterRadius);
ALGORITHM_PARAMETER_DECL(Distance);
ALGORITHM_PARAMETER_DECL(Layers);
ALGORITHM_PARAMETER_DECL(LayerStepSize);
ALGORITHM_PARAMETER_DECL(LevelOfDetail); 


  class SCISHARE ModelTMSCoilAlgorithm : public AlgorithmBase
  {
      public:
        ModelTMSCoilAlgorithm()
	{
	 addParameter(Parameters::Type,"");
	 addParameter(Parameters::FigureOf8CoilShape,2);
	 addParameter(Parameters::Current,1);
	 addParameter(Parameters::Rings,9);
	 addParameter(Parameters::InnerRadius,26);
	 addParameter(Parameters::OuterRadius,44);
	 addParameter(Parameters::Distance,2);
	 addParameter(Parameters::Layers,1);
	 addParameter(Parameters::LayerStepSize,1);
	 addParameter(Parameters::LevelOfDetail,6);
	}
	
        struct Args
        {
        	double current;
        	double coilRadiusInner;
        	double coilRadiusOuter;
            double coilDistance;
        	size_t coilLevelDetails;
            size_t coilLayers;
            double coilLayersStep;
            size_t rings;
        	int type;
        	
        	inline bool operator==(const Args& rhs)
        	{ 
    			return current == rhs.current && 
    			coilRadiusInner == rhs.coilRadiusInner && 
    			coilRadiusOuter == rhs.coilRadiusOuter && 
    			coilLevelDetails == rhs.coilLevelDetails &&
                coilDistance == rhs.coilDistance &&
                coilLayers == rhs.coilLayers &&
                coilLayersStep == rhs.coilLayersStep &&
                rings == rhs.rings &&
    			type == rhs.type;
        	}
    		inline bool operator!=( const Args& rhs){return !( (*this) == rhs );}
        };
        
        //! Generate the coil geometry
        AlgorithmOutput run(const AlgorithmInput& input) const override;       
        
    };
    
/*
  class SCISHARE ModelTMSCoilSingleAlgorithm : public AlgorithmBase
  {
      public:
        ModelTMSCoilSingleAlgorithm()
	{
	 addParameter(Parameters::FigureOf8CoilShape,2);
	 addParameter(Parameters::Current,1);
	 addParameter(Parameters::Radius,0.035);
	 addParameter(Parameters::Distance,0.002);
	 addParameter(Parameters::Layers,1);
	 addParameter(Parameters::LayerStepSize,0.001);
	 addParameter(Parameters::LevelOfDetail,6);
	}
	
        struct Args
        {
        	double wireCurrent;
        	double coilRadius;
            double coilDistanceOuter;
        	size_t coilLevelDetails;
            size_t coilLayers;
            double coilLayersStep;
        	int type;
        	
        	inline bool operator==(const Args& rhs)
        	{ 
    			return wireCurrent == rhs.wireCurrent && 
    			coilRadius == rhs.coilRadius && 
    			coilLevelDetails == rhs.coilLevelDetails &&
                coilDistanceOuter == rhs.coilDistanceOuter &&
                coilLayers == rhs.coilLayers &&
                coilLayersStep == rhs.coilLayersStep &&
    			type == rhs.type;
        	}
    		inline bool operator!=( const Args& rhs){return !( (*this) == rhs );}
        };
        
        //! Generate the coil geom
        bool run(FieldHandle& mesh, Args& args) const; 
        AlgorithmOutput run(const AlgorithmInput& input) const override;
    };
    
    class SCISHARE ModelTMSCoilSpiralAlgorithm : public AlgorithmBase
    {
      public:
        ModelTMSCoilSpiralAlgorithm()
        {
	// addParameter(Parameters::FigureOf8CoilShape,2);
	// addParameter(Parameters::Current,1);
	// addParameter(Parameters::Windings,9);
	// addParameter(Parameters::InnerRadius,0.026);
	// addParameter(Parameters::OuterRadius,0.044);
	// addParameter(Parameters::OuterDistance,0.002);
	// addParameter(Parameters::Layers,1);
	// addParameter(Parameters::LayerStepSize,0.001);
	// addParameter(Parameters::LevelOfDetail,6);
	}

        struct Args
        {
            double wireCurrent;
            size_t wireLoops;
            double coilRadiusInner;
            double coilRadiusOuter;
            double coilDistanceOuter;
            size_t coilLayers;
            double coilLayersStep;
            size_t coilLevelDetails;
            int type;
            
            inline bool operator==(const Args& rhs)
            { 
                return wireCurrent == rhs.wireCurrent && 
                wireLoops == rhs.wireLoops &&
                coilRadiusInner == rhs.coilRadiusInner && 
                coilRadiusOuter == rhs.coilRadiusOuter && 
                coilLevelDetails == rhs.coilLevelDetails &&
                coilLayers == rhs.coilLayers &&
                coilLayersStep == rhs.coilLayersStep &&
                coilDistanceOuter == rhs.coilDistanceOuter &&
                type == rhs.type;
            }
            inline bool operator!=( const Args& rhs){return !( (*this) == rhs );}
        };
        
        //! Generate the coil geom
        bool run(FieldHandle& mesh, Args& args) const; 
    };
    
    class SCISHARE ModelTMSCoilDipoleAlgorithm : public AlgorithmBase
    {
      public:
        ModelTMSCoilDipoleAlgorithm()
        {
         addParameter(Parameters::FigureOf8CoilShape,2);
	 addParameter(Parameters::Current,1);
	 addParameter(Parameters::Segments,5);
	 addParameter(Parameters::InnerRadius,0.026);
	 addParameter(Parameters::OuterRadius,0.044);
	 addParameter(Parameters::OuterDistance,0.002);
	 addParameter(Parameters::Layers,1);
	 addParameter(Parameters::LevelOfDetail,6); 
	}

        struct Args
        {
            double totalCurrent;
            size_t numberSegments;
            double coilRadiusInner;
            double coilRadiusOuter;
            double coilDistanceOuter;
            size_t coilLayers;
            size_t coilLevelDetails;
            int type;
            
            inline bool operator==(const Args& rhs)
            { 
                return 
                totalCurrent == rhs.totalCurrent && 
                numberSegments == rhs.numberSegments &&
                coilRadiusInner == rhs.coilRadiusInner && 
                coilRadiusOuter == rhs.coilRadiusOuter && 
                coilLevelDetails == rhs.coilLevelDetails &&
                coilDistanceOuter == rhs.coilDistanceOuter &&
                coilLayers == rhs.coilLayers &&
                type == rhs.type;
            }
            inline bool operator!=( const Args& rhs){return !( (*this) == rhs );}
        };
        
        //! Generate the coil geom
        bool run(FieldHandle& mesh, Args& args) const; 
	AlgorithmOutput run(const AlgorithmInput& input) const override;
    };
    
    */

}}}}

#endif
