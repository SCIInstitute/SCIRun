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


/// @todo Documentation Modules/Visualization/ShowFieldGlyphs.h

#ifndef MODULES_VISUALIZATION_SHOW_FIELD_GLYPHS_PORT_HANDLER_H
#define MODULES_VISUALIZATION_SHOW_FIELD_GLYPHS_PORT_HANDLER_H

#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Algorithms/Visualization/RenderFieldState.h>
#include <Core/Datatypes/Geometry.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Tensor.h>

namespace SCIRun{
  namespace Modules{
    namespace Visualization{

// This class takes in input information from the port and hands back coloring and scalar, vector, and tensor data
      class ShowFieldGlyphsPortHandler
      {
      private:
        enum FieldDataType
          {
           Scalar,
           Vector,
           Tensor,
           UNKNOWN = -1
          };

        const Dataflow::Networks::Module* module_;
        RenderState::InputPort secondaryVecInput;
        VField* p_vfld{ nullptr };
        VField* s_vfld{ nullptr };
        VField* t_vfld{ nullptr };
        FieldHandle pf_handle;
        FieldInformation pf_info;
        Graphics::Datatypes::ColorScheme colorScheme;
        RenderState::InputPort colorInput;
        Core::Datatypes::ColorRGB defaultColor;
        boost::optional<Core::Datatypes::ColorMapHandle> colorMap;
        Core::Datatypes::ColorMapHandle coordinateMap {nullptr}, textureMap {nullptr};
        boost::optional<Core::Geometry::Tensor> pinputTensor, sinputTensor, tinputTensor;
        boost::optional<Core::Geometry::Vector> pinputVector, sinputVector, tinputVector;
        boost::optional<double> pinputScalar, sinputScalar, tinputScalar;
        double current_index;
        bool colorMapGiven;
        bool secondaryFieldGiven, tertiaryFieldGiven;
        FieldDataType pf_data_type{ UNKNOWN }, sf_data_type{ UNKNOWN }, tf_data_type{ UNKNOWN };

        void getFieldData(int index);

        // Returns a color value to use for color maps
        Core::Datatypes::ColorRGB getColorMapVal(int index);

        // Returns color vector to be used for rgb conversion of tensors
        Core::Geometry::Vector getTensorColorVector(Core::Geometry::Tensor& t);

     public:
        ShowFieldGlyphsPortHandler(
                    const Dataflow::Networks::Module* mod_,
                    Dataflow::Networks::ModuleStateHandle state,
                    const RenderState renState,
                    FieldHandle pf,
                    boost::optional<FieldHandle> sf,
                    boost::optional<FieldHandle> tf,
                    boost::optional<Core::Datatypes::ColorMapHandle> pcolorMap,
                    boost::optional<Core::Datatypes::ColorMapHandle> scolorMap,
                    boost::optional<Core::Datatypes::ColorMapHandle> tcolorMap);

        // Verifies that data is valid. Run this after initialization
        void checkForErrors();

        void spiltColorMapToTextureAndCoordinates();

        Core::Datatypes::ColorMapHandle getTextureMap() {return textureMap;}

        // Returns color scheme that was set in render state
        Graphics::Datatypes::ColorScheme getColorScheme();

        // Returns color map
        Core::Datatypes::ColorMapHandle getColorMap();

        // Returns color of node
        Core::Datatypes::ColorRGB getNodeColor(int index);

        // Returns a vector with color in (x,y,z) format
        Core::Geometry::Vector getColorVector(int index);

        // Vector only. Returns a decimal number to use for the secondary vector parameter
        double getSecondaryVectorParameter(int index);

        // Return primary scalar
        double getPrimaryScalar(int index);

        // Return primary vector
        Core::Geometry::Vector getPrimaryVector(int index);

        // Return primary vector
        Core::Geometry::Tensor getPrimaryTensor(int index);

        // Get primary field information
        const FieldInformation getPrimaryFieldInfo();

        // Returns VMesh pointer
        VMesh* getMesh();

        // Returns VMesh Center Point
        Core::Geometry::Point* getMeshPoint();

        // Returns Facade of Primary field data
        Core::Datatypes::MeshTraits<VMesh>::MeshFacadeHandle getPrimaryFacade();
      };
    }
  }
}

#endif //MODULES_VISUALIZATION_SHOW_FIELD_GLYPHS_PORT_HANDLER_H
