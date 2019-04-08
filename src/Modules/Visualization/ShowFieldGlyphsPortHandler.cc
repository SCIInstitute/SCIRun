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
/// @todo Documentation Modules/Visualization/ShowFieldGlyphs.h

#include <Modules/Visualization/ShowFieldGlyphsPortHandler.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Core;
using namespace Core::Datatypes;
using namespace Geometry;
using namespace Graphics;
using namespace Graphics::Datatypes;
using namespace Dataflow::Networks;

namespace SCIRun{
  namespace Modules{
    namespace Visualization{
      ShowFieldGlyphsPortHandler::ShowFieldGlyphsPortHandler(
          const Dataflow::Networks::Module* mod_,
          ModuleStateHandle state,
          const RenderState renState,
          FieldHandle pf,
          boost::optional<FieldHandle> sf,
          boost::optional<FieldHandle> tf,
          FieldInformation pfieldinfo,
          boost::optional<FieldInformation> sfieldinfo,
          boost::optional<FieldInformation> tfieldinfo,
          boost::optional<ColorMapHandle> pcolorMap,
          boost::optional<ColorMapHandle> scolorMap,
          boost::optional<ColorMapHandle> tcolorMap)
      : pf_info(pfieldinfo),
        pf_handle(pf),
        sf_handle(sf.get()),
        sf_info(sfieldinfo.get()),
        tf_handle(tf.get()),
        tf_info(tfieldinfo.get())
      {
        //pf_handle = &pf;
        //sf_handle = &sf;
        //tf_handle = &tf;

        //pf_info = &pfieldinfo;
        //sf_info = &sfieldinfo;
        //tf_info = &tfieldinfo;

        //sfieldGiven = !!sf;
        //tfieldGiven = !!tf;

        renderState = &renState;

        // Set module for error throwing
        module_ = mod_;

        // Get Field and mesh from primary port
        p_vfld = (pf)->vfield();
        //FieldInformation (pf_info).pf);
        secondaryVecVal = renState.mVectorRadiusWidthInput;

        // Get info on coloring
        if(p_vfld->basis_order() < 0 || (*renderState).get(RenderState::USE_DEFAULT_COLOR))
          {
            colorScheme = ColorScheme::COLOR_UNIFORM;
          }
        else if((*renderState).get(RenderState::USE_COLORMAP))
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
            switch((*renderState).mColorInput)
              {
              case RenderState::InputPort::PRIMARY_PORT:
                if(pf_handle && pcolorMap)
                  colorMap = pcolorMap.get();
                else
                  {
                    std::invalid_argument("Primary Color Map input is required.");
                    return;
                  }
                break;
              case RenderState::InputPort::SECONDARY_PORT:
                if(sf_handle && scolorMap)
                  colorMap = scolorMap.get();
                else
                  {
                    std::invalid_argument("Secondary Field and Color Map input is required.");
                    return;
                  }
                break;
              case RenderState::InputPort::TERTIARY_PORT:
                if(tf_handle && tcolorMap)
                  colorMap = tcolorMap.get();
                else
                  {
                    std::invalid_argument("Tertiary Field and Color Map input is required.");
                    return;
                  }
                break;
              }
          }

        // Get secondary and tertiary field information if given
        if(*sf_handle)
          {
            s_vfld = (*sf_handle).get()->vfield();
            FieldInformation sf_info((*sf_handle).get());
          }
        if(*tf_handle)
          {
            t_vfld = (*tf_handle).get()->vfield();
            FieldInformation tf_info((*tf_handle).get());
          }
        current_index = -1;
      }

      // Gets scalar, vector, or tensor values from ports
      void ShowFieldGlyphsPortHandler::getFieldData(int index)
      {
        // Dont recalculate if we already have the value
        if(current_index == index)
          return;

        // Get input data from ports
        if ((pf_info).is_scalar())
          {
            double s;
            p_vfld->get_value(s, index);
            pinputScalar = s;
          }
        else if ((pf_info).is_vector())
          {
            Vector v;
            p_vfld->get_value(v, index);
            pinputVector = v;
          }
        else
          {
            Tensor t;
            p_vfld->get_value(t, index);
            pinputTensor = t;
          }
        if(sf_info)
          {
            if ((sf_info).get().is_scalar())
              {
                double s;
                s_vfld->get_value(s, index);
                sinputScalar = s;
              }
            else if ((sf_info).get().is_vector())
              {
                Vector v;
                s_vfld->get_value(v, index);
                sinputVector = v;
              }
            else
              {
                Tensor t;
                s_vfld->get_value(t, index);
                sinputTensor = t;
              }
          }
        if(tf_info)
          {
            if ((tf_info).get().is_scalar())
              {
                double s;
                t_vfld->get_value(s, index);
                tinputScalar = s;
              }
            else if ((tf_info).get().is_vector())
              {
                Vector v;
                t_vfld->get_value(v, index);
                tinputVector = v;
              }
            else
              {
                Tensor t;
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
        //        ColorMapHandle map = colorMap;
        switch((*renderState).mColorInput)
          {
          case RenderState::InputPort::PRIMARY_PORT:
            if((pf_info).is_scalar())
              {
                colorMapVal = colorMap->valueToColor(pinputScalar.get());
              }
            else if((pf_info).is_vector())
              {
                colorMapVal = colorMap->valueToColor(pinputVector.get());
              }
            else
              {
                colorMapVal = colorMap->valueToColor(pinputTensor.get());
              }
            break;
          case RenderState::InputPort::SECONDARY_PORT:
            if((sf_info).get().is_scalar())
              {
                colorMapVal = colorMap->valueToColor(sinputScalar.get());
              }
            else if((sf_info).get().is_vector())
              {
                colorMapVal = colorMap->valueToColor(sinputVector.get());
              }
            else
              {
                colorMapVal = colorMap->valueToColor(sinputTensor.get());
              }
            break;
          case RenderState::InputPort::TERTIARY_PORT:
            if((tf_info).get().is_scalar())
              {
                colorMapVal = colorMap->valueToColor(tinputScalar.get());
              }
            else if((tf_info).get().is_vector())
              {
                colorMapVal = colorMap->valueToColor(tinputVector.get());
              }
            else
              {
                colorMapVal = colorMap->valueToColor(tinputTensor.get());
              }
            break;
          }
        return colorMapVal;
      }

      ColorScheme ShowFieldGlyphsPortHandler::getColorScheme()
      {
        return colorScheme;
      }

      // Returns the Color Vector for RGB Conversion based on the Input Port
      Vector ShowFieldGlyphsPortHandler::getColorVector(int index)
      {
        getFieldData(index);

        Vector colorVector;
        switch((*renderState).mColorInput)
          {
          case RenderState::InputPort::PRIMARY_PORT:
            if((pf_info).is_scalar())
              {
                throw std::invalid_argument("Primary Field input cannot be a scalar for RGB Conversion.");
                //return;
              }
            else if((pf_info).is_vector())
              {
                colorVector = pinputVector.get();
              }
            else
              {
                Vector eigvec1, eigvec2, eigvec3;
                pinputTensor.get().get_eigenvectors(eigvec1, eigvec2, eigvec3);
                colorVector = eigvec1;
              }
            break;
          case RenderState::InputPort::SECONDARY_PORT:
            if((sf_info).get().is_scalar())
              {
                throw std::invalid_argument("Secondary Field input cannot be a scalar for RGB Conversion.");
                //return;
              }
            else if((sf_info).get().is_vector())
              {
                colorVector = sinputVector.get();
              }
            else
              {
                Vector eigvec1, eigvec2, eigvec3;
                sinputTensor.get().get_eigenvectors(eigvec1, eigvec2, eigvec3);
                colorVector = eigvec1;
              }
            break;
          case RenderState::InputPort::TERTIARY_PORT:
            if((tf_info).get().is_scalar())
              {
                throw std::invalid_argument("Tertiary Field input cannot be a scalar for RGB Conversion.");
                //return;
              }
            else if((tf_info).get().is_vector())
              {
                colorVector = tinputVector.get();
              }
            else
              {
                Vector eigvec1, eigvec2, eigvec3;
                tinputTensor.get().get_eigenvectors(eigvec1, eigvec2, eigvec3);
                colorVector = eigvec1;
              }
            break;
          }
        return colorVector;
      }

      ColorRGB ShowFieldGlyphsPortHandler::getNodeColor(int index)
      {
        getFieldData(index);

        ColorRGB node_color;
        switch(colorScheme)
          {
          case ColorScheme::COLOR_UNIFORM:
            node_color = (*renderState).defaultColor;
            break;
          case ColorScheme::COLOR_MAP:
            node_color = getColorMapVal(index);
            break;
          case ColorScheme::COLOR_IN_SITU:
            Vector colorVector;
            colorVector = getColorVector(index).normal();
            node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
            break;
          }
        return node_color;
      }

      double ShowFieldGlyphsPortHandler::getSecondaryVectorParameter(int index)
      {
        getFieldData(index);

        double val;
        switch(secondaryVecVal)
          {
            // Primary can only be vector for this function
          case RenderState::InputPort::PRIMARY_PORT:
            val = pinputVector.get().length();
            break;
          case RenderState::InputPort::SECONDARY_PORT:
            if(sf_handle)
              {
                if((sf_info).get().is_scalar())
                  {
                    val = sinputScalar.get();
                  }
                else if((sf_info).get().is_vector())
                  {
                    val = sinputVector.get().length();
                  }
                else
                  {
                    val = sinputTensor.get().magnitude();
                  }
              }
            else
              {
                std::invalid_argument("Secondary Field input is required for Secondary Vector Parameter.");
                return 0.0;
              }
            break;
          case RenderState::InputPort::TERTIARY_PORT:
            if(tf_handle)
              {
                if((tf_info).get().is_scalar())
                  {
                    val = tinputScalar.get();
                  }
                else if((tf_info).get().is_vector())
                  {
                    val = tinputVector.get().length();
                  }
                else
                  {
                    val = tinputTensor.get().magnitude();
                  }
              }
            else
              {
                std::invalid_argument("Tertiary Field input is required for Secondary Vector Parameter.");
                return 0.0;
              }
            break;
          default:
            val = pinputVector.get().length();
            break;
          }
        return val;
      }

      Vector ShowFieldGlyphsPortHandler::getPrimaryVector(int index)
      {
        getFieldData(index);

        return pinputVector.get();
      }

      const FieldInformation ShowFieldGlyphsPortHandler::getPrimaryFieldInfo()
      {
        return (pf_info);
      }

      ColorMapHandle ShowFieldGlyphsPortHandler::getColorMap()
      {
        return colorMap;
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
    }
  }
}
