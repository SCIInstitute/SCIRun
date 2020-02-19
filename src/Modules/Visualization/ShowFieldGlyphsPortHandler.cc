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

#include <Modules/Visualization/ShowFieldGlyphsPortHandler.h>
#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Core;
using namespace Core::Datatypes;
using namespace Graphics;
using namespace Graphics::Datatypes;
using namespace Dataflow::Networks;

namespace SCIRun{
  namespace Modules{
    namespace Visualization{
      ShowFieldGlyphsPortHandler::ShowFieldGlyphsPortHandler(
          const Dataflow::Networks::Module* mod,
          ModuleStateHandle state,
          const RenderState renState,
          FieldHandle pf,
          boost::optional<FieldHandle> sf,
          boost::optional<FieldHandle> tf,
          boost::optional<ColorMapHandle> pcolorMap,
          boost::optional<ColorMapHandle> scolorMap,
          boost::optional<ColorMapHandle> tcolorMap)
        : module_(mod), pf_handle(pf), pf_info(pf)
      {
        // Save field info
        p_vfld = (pf)->vfield();
        if(pf_info.is_scalar())
          {
            pf_data_type = FieldDataType::Scalar;
          }
        else if(pf_info.is_vector())
          {
            pf_data_type = FieldDataType::Vector;
          }
        else
          {
            pf_data_type = FieldDataType::Tensor;
          }
        if(sf)
          {
            s_vfld = (sf.get())->vfield();
            secondaryFieldGiven = true;
            FieldInformation sf_info(sf.get());
            if(sf_info.is_scalar())
              {
                sf_data_type = FieldDataType::Scalar;
              }
            else if(sf_info.is_vector())
              {
                sf_data_type = FieldDataType::Vector;
              }
            else
              {
                sf_data_type = FieldDataType::Tensor;
              }
          }
        else
          {
            secondaryFieldGiven = false;
          }
        if(tf)
          {
            t_vfld = (tf.get())->vfield();
            tertiaryFieldGiven = true;
            FieldInformation tf_info(tf.get());
            if(tf_info.is_scalar())
              {
                tf_data_type = FieldDataType::Scalar;
              }
            else if(tf_info.is_vector())
              {
                tf_data_type = FieldDataType::Vector;
              }
            else
              {
                tf_data_type = FieldDataType::Tensor;
              }
          }
        else
          {
            tertiaryFieldGiven = false;
          }

        // Get Field and mesh from primary port
        p_vfld = pf->vfield();
        secondaryVecInput = renState.mSecondaryVectorParameterInput;

        // Get info on coloring
        if((p_vfld->basis_order() < 0 && pf->vmesh()->dimensionality() != 0)
           || renState.get(RenderState::USE_DEFAULT_COLOR))
          {
            colorScheme = ColorScheme::COLOR_UNIFORM;
          }
        else if(renState.get(RenderState::USE_COLORMAP))
          {
            colorScheme = ColorScheme::COLOR_MAP;
          }
        else
          {
            colorScheme = ColorScheme::COLOR_IN_SITU;
          }

        // If color map was picked, set the chosen color map
        if(colorScheme == ColorScheme::COLOR_MAP)
        {
          switch(renState.mColorInput)
          {
            case RenderState::InputPort::PRIMARY_PORT:
              if(pcolorMap)
              {
                colorMap = pcolorMap;
                colorMapGiven = true;
              }
              else
              {
                colorMapGiven = false;
              }
              break;
            case RenderState::InputPort::SECONDARY_PORT:
              if(scolorMap)
              {
                colorMap = scolorMap;
                colorMapGiven = true;
              }
              else
              {
                colorMapGiven = false;
              }
              break;
            case RenderState::InputPort::TERTIARY_PORT:
              if(tcolorMap)
              {
                colorMap = tcolorMap;
                colorMapGiven = true;
              }
              else
              {
                colorMapGiven = false;
              }
              break;
            default:
              throw std::invalid_argument("Selected port was not primary, secondary, or tertiary.");
              break;
          }

          spiltColorMapToTextureAndCoordinates();
        }

        // Get color input type from render state
        colorInput = renState.mColorInput;

        // Get default color
        defaultColor = renState.defaultColor;

        current_index = -1;
      }

      // Gets scalar, vector, or tensor values from ports
      void ShowFieldGlyphsPortHandler::getFieldData(int index)
      {
        // Dont recalculate if we already have the value
        if(current_index == index)
          return;

        if (p_vfld)
        {
          // Get input data from ports
          if (pf_data_type == FieldDataType::Scalar)
          {
            double s;
            p_vfld->get_value(s, index);
            pinputScalar = s;
          }
          else if (pf_data_type == FieldDataType::Vector)
          {
            Geometry::Vector v;
            p_vfld->get_value(v, index);
            pinputVector = v;
          }
          else if (pf_data_type == FieldDataType::Tensor)
          {
            Geometry::Tensor t;
            p_vfld->get_value(t, index);
            pinputTensor = t;
          }
        }
        if (s_vfld)
        {
          if (sf_data_type == FieldDataType::Scalar)
          {
            double s;
            s_vfld->get_value(s, index);
            sinputScalar = s;
          }
          else if (sf_data_type == FieldDataType::Vector)
          {
            Geometry::Vector v;
            s_vfld->get_value(v, index);
            sinputVector = v;
          }
          else if (sf_data_type == FieldDataType::Tensor)
          {
            Geometry::Tensor t;
            s_vfld->get_value(t, index);
            sinputTensor = t;
          }
        }
        if (t_vfld)
        {
          if (tf_data_type == FieldDataType::Scalar)
          {
            double s;
            t_vfld->get_value(s, index);
            tinputScalar = s;
          }
          else if (tf_data_type == FieldDataType::Vector)
          {
            Geometry::Vector v;
            t_vfld->get_value(v, index);
            tinputVector = v;
          }
          else if (tf_data_type == FieldDataType::Tensor)
          {
            Geometry::Tensor t;
            t_vfld->get_value(t, index);
            tinputTensor = t;
          }
        }
        // Set current index so it doesn't rerun for the same index
        current_index = index;
      }

      // Returns the color map value based on the Input Port
      ColorRGB ShowFieldGlyphsPortHandler::getColorMapVal(int index)
      {
        getFieldData(index);

        ColorRGB colorMapVal;
        switch(colorInput)
        {
          case RenderState::InputPort::PRIMARY_PORT:
            switch(pf_data_type)
            {
              case FieldDataType::Scalar:
                colorMapVal = colorMap.get()->valueToColor(pinputScalar.get());
                break;
              case FieldDataType::Vector:
                colorMapVal = colorMap.get()->valueToColor(pinputVector.get());
                break;
              case FieldDataType::Tensor:
                colorMapVal = colorMap.get()->valueToColor(pinputTensor.get());
                break;
              default:
                throw std::invalid_argument("Primary color map did not find scalar, vector, or tensor data.");
                break;
            }
            break;
          case RenderState::InputPort::SECONDARY_PORT:
            switch(sf_data_type)
            {
              case FieldDataType::Scalar:
                colorMapVal = colorMap.get()->valueToColor(sinputScalar.get());
                break;
              case FieldDataType::Vector:
                colorMapVal = colorMap.get()->valueToColor(sinputVector.get());
                break;
              case FieldDataType::Tensor:
                colorMapVal = colorMap.get()->valueToColor(sinputTensor.get());
                break;
              default:
                throw std::invalid_argument("Secondary color map did not find scalar, vector, or tensor data.");
                break;
            }
            break;
          case RenderState::InputPort::TERTIARY_PORT:
            switch(tf_data_type)
            {
              case FieldDataType::Scalar:
                colorMapVal = colorMap.get()->valueToColor(tinputScalar.get());
                break;
              case FieldDataType::Vector:
                colorMapVal = colorMap.get()->valueToColor(tinputVector.get());
                break;
              case FieldDataType::Tensor:
                colorMapVal = colorMap.get()->valueToColor(tinputTensor.get());
                break;
              default:
                throw std::invalid_argument("Tertiary color map did not find scalar, vector, or tensor data.");
                break;
            }
            break;
          default:
            throw std::invalid_argument("Color map selection was not given a primary, secondary, or tertiary port.");
            break;
        }
        return colorMapVal;
      }

      // Verifies that data is valid. Run this after initialization
      void ShowFieldGlyphsPortHandler::checkForErrors()
      {
        // Make sure color map port and correpsonding field data is given for chosen color map
        if(colorScheme == ColorScheme::COLOR_MAP)
        {
          switch(colorInput)
          {
            case RenderState::InputPort::PRIMARY_PORT:
              if(!colorMap)
              {
                throw std::invalid_argument("Primary Color Map input is required.");
              }
              break;
            case RenderState::InputPort::SECONDARY_PORT:
              if(!(secondaryFieldGiven && colorMap))
              {
                throw std::invalid_argument("Secondary Field and Color Map input is required.");
              }
              if(s_vfld->num_values() < p_vfld->num_values())
              {
                throw std::invalid_argument("Secondary Field input cannot have a smaller size than the Primary Field input.");
              }
              break;
            case RenderState::InputPort::TERTIARY_PORT:
              if(!(tertiaryFieldGiven && colorMap))
              {
                throw std::invalid_argument("Tertiary Field and Color Map input is required.");
              }
              if(t_vfld->num_values() < p_vfld->num_values())
              {
                throw std::invalid_argument("Tertiary Field input cannot have a smaller size than the Primary Field input.");
              }
              break;
            default:
              throw std::invalid_argument("Must select a primary, secondary, or tertiary port for color map input.");
              break;
          }
        }
        // Make sure scalar is not given for rgb conversion
        else if(colorScheme == ColorScheme::COLOR_IN_SITU)
          {
            switch(colorInput)
              {
              case RenderState::InputPort::PRIMARY_PORT:
                if(pf_data_type == FieldDataType::Scalar)
                  {
                    throw std::invalid_argument("Primary Field input cannot be a scalar for RGB Conversion.");
                  }
                break;
              case RenderState::InputPort::SECONDARY_PORT:
                if(sf_data_type == FieldDataType::Scalar)
                  {
                    throw std::invalid_argument("Secondary Field input cannot be a scalar for RGB Conversion.");
                  }
                if(s_vfld->num_values() < p_vfld->num_values())
                  {
                    throw std::invalid_argument("Secondary Field input cannot have a smaller size than the Primary Field input.");
                  }
                break;
              case RenderState::InputPort::TERTIARY_PORT:
                if(tf_data_type == FieldDataType::Scalar)
                  {
                    throw std::invalid_argument("Tertiary Field input cannot be a scalar for RGB Conversion.");
                  }
                if(t_vfld->num_values() < p_vfld->num_values())
                  {
                    throw std::invalid_argument("Tertiary Field input cannot have a smaller size than the Primary Field input.");
                  }
                break;
              default:
                throw std::invalid_argument("Must select a primary, secondary, or tertiary port for rgb conversion input.");
                break;
              }
          }

        // Make sure port is given for chosen secondary Vector input
         switch(secondaryVecInput)
          {
          case RenderState::InputPort::PRIMARY_PORT:
            // Primary already present so no possible errors
            break;
          case RenderState::InputPort::SECONDARY_PORT:
            if(!secondaryFieldGiven)
              {
                throw std::invalid_argument("Secondary Field input is required for Secondary Vector Parameter.");
              }
            if(s_vfld->num_values() < p_vfld->num_values())
              {
                throw std::invalid_argument("Secondary Field input cannot have a smaller size than the Primary Field input.");
              }
            break;
          case RenderState::InputPort::TERTIARY_PORT:
            if(!tertiaryFieldGiven)
              {
                throw std::invalid_argument("Tertiary Field input is required for Secondary Vector Parameter.");
              }
            if(t_vfld->num_values() < p_vfld->num_values())
              {
                throw std::invalid_argument("Tertiary Field input cannot have a smaller size than the Primary Field input.");
              }
            break;
          default:
            throw std::invalid_argument("Must select a primary, secondary, or tertiary port for secondary vector input.");
            break;
          }
      }

      // Returns color scheme that was set in render state
      ColorScheme ShowFieldGlyphsPortHandler::getColorScheme()
      {
        return colorScheme;
      }

      // Returns the Color Vector for RGB Conversion based on the Input Port
      Geometry::Vector ShowFieldGlyphsPortHandler::getColorVector(int index)
      {
        getFieldData(index);

        Geometry::Vector colorVector;
        switch(colorInput)
          {
          case RenderState::InputPort::PRIMARY_PORT:
            if(pf_data_type == FieldDataType::Vector)
              {
                colorVector = pinputVector.get();
              }
            else
              {
                colorVector = getTensorColorVector(pinputTensor.get());
              }
            break;
          case RenderState::InputPort::SECONDARY_PORT:
            if(sf_data_type == FieldDataType::Vector)
              {
                colorVector = sinputVector.get();
              }
            else
              {
                colorVector = getTensorColorVector(sinputTensor.get());
              }
            break;
          case RenderState::InputPort::TERTIARY_PORT:
            if(tf_data_type == FieldDataType::Vector)
              {
                colorVector = tinputVector.get();
              }
            else
              {
                colorVector = getTensorColorVector(tinputTensor.get());
              }
            break;
          default:
            throw std::invalid_argument("Must select a primary, secondary, or tertiary port for selecting color vector.");
            break;
          }
        return colorVector;
      }

      // Returns color vector for tensor that are using rgb conversion
      Geometry::Vector ShowFieldGlyphsPortHandler::getTensorColorVector(Geometry::Tensor& t)
      {
        Geometry::Vector colorVector;
        double eigval1, eigval2, eigval3;
        t.get_eigenvalues(eigval1, eigval2, eigval3);

        if(eigval1 == eigval2 && eigval1 != eigval3){
          Geometry::Vector eigvec3_norm = t.get_eigenvector3().normal();
          Geometry::Vector xCross = Cross(eigvec3_norm, Geometry::Vector(1,0,0));
          Geometry::Vector yCross = Cross(eigvec3_norm, Geometry::Vector(0,1,0));
          Geometry::Vector zCross = Cross(eigvec3_norm, Geometry::Vector(0,0,1));
          xCross.normalize();
          yCross.normalize();
          zCross.normalize();

          double epsilon = pow(2, -52);
          if(std::abs(Dot(xCross, yCross)) > (1-epsilon)){
            colorVector = xCross;
          }
          else if(std::abs(Dot(yCross, zCross)) > (1-epsilon)){
            colorVector = yCross;
          }
          else if(std::abs(Dot(xCross, zCross)) > (1-epsilon)){
            colorVector = zCross;
          }
          else{
            colorVector = t.get_eigenvector1();
          }
        } else{
          colorVector = t.get_eigenvector1();
        }
        colorVector = Abs(colorVector);
        colorVector.normalize();
        return colorVector;
      }

      // Returns color of node
      ColorRGB ShowFieldGlyphsPortHandler::getNodeColor(int index)
      {
        getFieldData(index);

        ColorRGB node_color;
        switch(colorScheme)
          {
          case ColorScheme::COLOR_UNIFORM:
            node_color = defaultColor;
            break;
          case ColorScheme::COLOR_MAP:
            if(colorMapGiven)
              {
                node_color = getColorMapVal(index);
              }
            break;
          case ColorScheme::COLOR_IN_SITU:
          {
            Geometry::Vector colorVector;
            colorVector = getColorVector(index).normal();
            node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
            break;
          }
          default:
            throw std::invalid_argument("Must select a primary, secondary, or tertiary port for selecting color of node.");
            break;
          }
        return node_color;
      }

      // Vector only. Returns a decimal number to use for the secondary vector parameter
      double ShowFieldGlyphsPortHandler::getSecondaryVectorParameter(int index)
      {
        getFieldData(index);

        double val;
        switch(secondaryVecInput)
          {
            // Primary can only be vector for this function
          case RenderState::InputPort::PRIMARY_PORT:
            val = pinputVector.get().length();
            break;
          case RenderState::InputPort::SECONDARY_PORT:
            if(sf_data_type == FieldDataType::Scalar)
              {
                val = sinputScalar.get();
              }
            else if(sf_data_type == FieldDataType::Vector)
              {
                val = sinputVector.get().length();
              }
            else
              {
                val = sinputTensor.get().magnitude();
              }
            break;
          case RenderState::InputPort::TERTIARY_PORT:
            if(tf_data_type == FieldDataType::Scalar)
              {
                val = tinputScalar.get();
              }
            else if(tf_data_type == FieldDataType::Vector)
              {
                val = tinputVector.get().length();
              }
            else
              {
                val = tinputTensor.get().magnitude();
              }
            break;
          default:
            val = pinputVector.get().length();
            break;
          }
        return abs(val);
      }

      // Return primary scalar
      double ShowFieldGlyphsPortHandler::getPrimaryScalar(int index)
      {
        getFieldData(index);

        return pinputScalar.get();
      }

      // Return primary vector
      Geometry::Vector ShowFieldGlyphsPortHandler::getPrimaryVector(int index)
      {
        getFieldData(index);

        return pinputVector.get();
      }

      // Return primary vector
      Geometry::Tensor ShowFieldGlyphsPortHandler::getPrimaryTensor(int index)
      {
        getFieldData(index);

        return pinputTensor.get();
      }

      // Get primary field information
      const FieldInformation ShowFieldGlyphsPortHandler::getPrimaryFieldInfo()
      {
        return (pf_info);
      }

      // Returns color map
      ColorMapHandle ShowFieldGlyphsPortHandler::getColorMap()
      {
        return colorMap.get();
      }

      // Returns VMesh pointer
      VMesh* ShowFieldGlyphsPortHandler::getMesh()
      {
       return pf_handle->vmesh();
      }

      // Returns Facade of Primary field data
      MeshTraits<VMesh>::MeshFacadeHandle ShowFieldGlyphsPortHandler::getPrimaryFacade()
      {
       return pf_handle->mesh()->getFacade();
      }

      void ShowFieldGlyphsPortHandler::spiltColorMapToTextureAndCoordinates()
      {
        ColorMapHandle realColorMap = nullptr;

        if(colorMap) realColorMap = colorMap.get();
        else realColorMap = StandardColorMapFactory::create();

        textureMap = StandardColorMapFactory::create(
          realColorMap->getColorData(), realColorMap->getColorMapName(),
          realColorMap->getColorMapResolution(), realColorMap->getColorMapShift(),
          realColorMap->getColorMapInvert(), 0.5, 1.0, realColorMap->getAlphaLookup());

        coordinateMap = StandardColorMapFactory::create("Grayscale", 256, 0, false,
          realColorMap->getColorMapRescaleScale(), realColorMap->getColorMapRescaleShift());

        colorMap = coordinateMap;
      }

    }
  }
}
