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

#include <Graphics/Glyphs/GlyphGeom.h>
#include <Core/Math/MiscMath.h>
#include <Core/GeometryPrimitives/Transform.h>

using namespace SCIRun;
using namespace Graphics;
using namespace Datatypes;
using namespace Core::Geometry;
using namespace Core::Datatypes;

GlyphGeom::GlyphGeom() : numVBOElements_(0), lineIndex_(0)
{

}

void GlyphGeom::getBufferInfo(int64_t& numVBOElements, std::vector<Vector>& points, std::vector<Vector>& normals,
                              std::vector<ColorRGB>& colors, std::vector<uint32_t>& indices)
{
  numVBOElements = numVBOElements_;
  points = points_;
  normals = normals_;
  colors = colors_;
  indices = indices_;
}

void GlyphGeom::buildObject(GeometryHandle geom, const std::string& uniqueNodeID, const bool isTransparent, const double transparencyValue,
  const ColorScheme& colorScheme, RenderState state, const SpireIBO::PRIMITIVE& primIn, const BBox& bbox)
{
  std::string vboName = uniqueNodeID + "VBO";
  std::string iboName = uniqueNodeID + "IBO";
  std::string passName = uniqueNodeID + "Pass";

  bool useTriangles = primIn == SpireIBO::TRIANGLES;

  // Construct VBO.
  std::string shader = "Shaders/UniformColor";
  std::vector<SpireVBO::AttributeData> attribs;
  attribs.push_back(SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  if (useTriangles)
    attribs.push_back(SpireVBO::AttributeData("aNormal", 3 * sizeof(float)));
  RenderType renderType = RENDER_VBO_IBO;

  //ColorScheme colorScheme = COLOR_UNIFORM;

  std::vector<SpireSubPass::Uniform> uniforms;
  if (isTransparent)
    uniforms.push_back(SpireSubPass::Uniform("uTransparency", (float)(transparencyValue)));
  // TODO: add colormapping options
  if (colorScheme == COLOR_MAP)
  {
    attribs.push_back(SpireVBO::AttributeData("aColor", 4 * sizeof(float)));
    if (useTriangles)
    {
      shader = "Shaders/DirPhongCMap";
      uniforms.push_back(SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(SpireSubPass::Uniform("uSpecularPower", 32.0f));
    }
    else
    {
      shader = "Shaders/ColorMap";
    }
  }
  else if (colorScheme == COLOR_IN_SITU)
  {
    attribs.push_back(SpireVBO::AttributeData("aColor", 4 * sizeof(float)));
    if (useTriangles)
    {
      shader = "Shaders/DirPhongInSitu";
      uniforms.push_back(SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(SpireSubPass::Uniform("uSpecularPower", 32.0f));
    }
    else
    {
      shader = "Shaders/InSituColor";
    }
  }
  else if (colorScheme == COLOR_UNIFORM)
  {
    ColorRGB dft = state.defaultColor;
    if (useTriangles)
    {
      if (geom->isClippable())
        shader = "Shaders/DirPhong";
      else
        shader = "Shaders/DirPhongNoClipping";
      uniforms.push_back(SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(SpireSubPass::Uniform("uDiffuseColor",
        glm::vec4(dft.r(), dft.g(), dft.b(), (float)transparencyValue)));
      uniforms.push_back(SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(SpireSubPass::Uniform("uSpecularPower", 32.0f));
    }
    else
    {
      uniforms.emplace_back("uColor", glm::vec4(dft.r(), dft.g(), dft.b(), (float)transparencyValue));
    }
  }

  uint32_t iboSize = 0;
  uint32_t vboSize = 0;

  vboSize = static_cast<uint32_t>(points_.size()) * 3 * sizeof(float);
  vboSize += static_cast<uint32_t>(normals_.size()) * 3 * sizeof(float);
  if (colorScheme == COLOR_IN_SITU || colorScheme == COLOR_MAP)
    vboSize += static_cast<uint32_t>(colors_.size()) * 4 * sizeof(float); //RGBA
  iboSize = static_cast<uint32_t>(indices_.size()) * sizeof(uint32_t);
  /// \todo To reduce memory requirements, we can use a 16bit index buffer.

  /// \todo To further reduce a large amount of memory, get rid of the index
  ///       buffer and use glDrawArrays to render without an IBO. An IBO is
  ///       a waste of space.
  ///       http://www.opengl.org/sdk/docs/man3/xhtml/glDrawArrays.xml

  /// \todo Switch to unique_ptrs and move semantics.
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr(new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr(new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));

  // Accessing the pointers like this is contrived. We only do this for
  // speed since we will be using the pointers in a tight inner loop.
  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer = iboBufferSPtr.get();
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer = vboBufferSPtr.get();

  //write to the IBO/VBOs

  for (auto a : indices_)
    iboBuffer->write(a);

  for (size_t i = 0; i < points_.size(); i++)
  {
    // Write first point on line
    vboBuffer->write(static_cast<float>(points_.at(i).x()));
    vboBuffer->write(static_cast<float>(points_.at(i).y()));
    vboBuffer->write(static_cast<float>(points_.at(i).z()));
    // Write normal
    if (normals_.size() == points_.size())
    {
      vboBuffer->write(static_cast<float>(normals_.at(i).x()));
      vboBuffer->write(static_cast<float>(normals_.at(i).y()));
      vboBuffer->write(static_cast<float>(normals_.at(i).z()));
    }
    if (colorScheme == COLOR_MAP || colorScheme == COLOR_IN_SITU)
    {
      vboBuffer->write(static_cast<float>(colors_.at(i).r()));
      vboBuffer->write(static_cast<float>(colors_.at(i).g()));
      vboBuffer->write(static_cast<float>(colors_.at(i).b()));
      vboBuffer->write(static_cast<float>(colors_.at(i).a()));
      //vboBuffer->write(static_cast<float>(1.f));
    } // no color writing otherwise
  }

  // If true, then the VBO will be placed on the GPU. We don't want to place
  // VBOs on the GPU when we are generating rendering lists.
  SpireVBO geomVBO(vboName, attribs, vboBufferSPtr, numVBOElements_, bbox, true);

  // Construct IBO.
  SpireIBO geomIBO(iboName, primIn, sizeof(uint32_t), iboBufferSPtr);

  state.set(RenderState::IS_ON, true);
  state.set(RenderState::HAS_DATA, true);

  SpireText text;

  // Construct Pass.
  SpireSubPass pass(passName, vboName, iboName, shader, colorScheme, state, renderType, geomVBO, geomIBO, text);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  geom->mVBOs.push_back(geomVBO);
  geom->mIBOs.push_back(geomIBO);
  geom->mPasses.push_back(pass);
}

void GlyphGeom::addArrow(const Point& p1, const Point& p2, double radius, double resolution,
  const ColorRGB& color1, const ColorRGB& color2)
{
  double ratio = 0.5;

  Point mid(ratio * (p1.x() + p2.x()), ratio * (p1.y() + p2.y()), ratio * (p1.z() + p2.z()));

  generateCylinder(p1, mid, radius / 6.0, radius / 6.0, resolution, color1, color2, numVBOElements_, points_, normals_, indices_, colors_);
  generateCylinder(mid, p2, radius, 0.0, resolution, color1, color2, numVBOElements_, points_, normals_, indices_, colors_);
}

void GlyphGeom::addSphere(const Point& p, double radius, double resolution, const ColorRGB& color)
{
  generateSphere(p, radius, radius, resolution, color, numVBOElements_, points_, normals_, indices_, colors_);
}

void GlyphGeom::addEllipsoid(const Point& p, double radius1, double radius2, double resolution, const ColorRGB& color)
{
  generateEllipsoid(p, radius1, radius2, resolution, color, numVBOElements_, points_, normals_, indices_, colors_);
}

void GlyphGeom::addCylinder(const Point p1, const Point& p2, double radius, double resolution,
                            const ColorRGB& color1, const ColorRGB& color2)
{  
  generateCylinder(p1, p2, radius, radius, resolution, color1, color2, numVBOElements_, points_, normals_, indices_, colors_);
}

void GlyphGeom::addCone(const Point p1, const Point& p2, double radius, double resolution,
  const ColorRGB& color1, const ColorRGB& color2)
{
  //std::cout << "p1: " << p1 << " p2 " << p2 << " radius: " << radius << " resolution: " << resolution << " color1: " << color1 << " color2: " << color2 << std::endl;
  generateCylinder(p1, p2, radius, 0.0, resolution, color1, color2, numVBOElements_, points_, normals_, indices_, colors_);
}

void GlyphGeom::addClippingPlane(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2,
  const Core::Geometry::Point& p3, const Core::Geometry::Point& p4, double radius, double resolution,
  const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2)
{
  addSphere(p1, radius, resolution, color1);
  addSphere(p2, radius, resolution, color1);
  addSphere(p3, radius, resolution, color1);
  addSphere(p4, radius, resolution, color1);
  addCylinder(p1, p2, radius, resolution, color1, color2);
  addCylinder(p2, p3, radius, resolution, color1, color2);
  addCylinder(p3, p4, radius, resolution, color1, color2);
  addCylinder(p4, p1, radius, resolution, color1, color2);
}

void GlyphGeom::addPlane(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2,
  const Core::Geometry::Point& p3, const Core::Geometry::Point& p4,
  const Core::Datatypes::ColorRGB& color1)
{
  generatePlane(p1, p2, p3, p4, color1, numVBOElements_, points_, normals_, indices_, colors_);
}

void GlyphGeom::addLine(Point p1, const Point& p2, const ColorRGB& color1, const ColorRGB& color2)
{
  generateLine(p1, p2, color1, color2, numVBOElements_, points_, indices_, colors_);
}

void GlyphGeom::addNeedle(Point p1, const Point& p2, const ColorRGB& color1, const ColorRGB& color2)
{
  Point mid(0.5 * (p1.x() + p2.x()), 0.5 * (p1.y() + p2.y()), 0.5 * (p1.z() + p2.z()));
  ColorRGB endColor(color2.r(), color2.g(), color2.b(), 0.5);
  generateLine(p1, mid, color1, endColor, numVBOElements_, points_, indices_, colors_);
  generateLine(mid, p2, color1, endColor, numVBOElements_, points_, indices_, colors_);
}

void GlyphGeom::addPoint(const Point& p, const ColorRGB& color)
{
  generatePoint(p, color, numVBOElements_, points_, indices_, colors_);
}

void GlyphGeom::generateCylinder(const Point& p1, const Point& p2, double radius1,
  double radius2, double resolution, const ColorRGB& color1, const ColorRGB& color2,
  int64_t& numVBOElements, std::vector<Vector>& points, std::vector<Vector>& normals,
  std::vector<uint32_t>& indices, std::vector<ColorRGB>& colors)
{
  double num_strips = resolution;
  if (num_strips < 0) num_strips = 20.0;
  double r1 = radius1 < 0 ? 1.0 : radius1;
  double r2 = radius2 < 0 ? 1.0 : radius2;

  //generate triangles for the cylinders.
  Vector n((p1 - p2).normal()), u = (10 * n + Vector(10, 10, 10)).normal();
  Vector crx = Cross(u, n).normal();
  u = Cross(crx, n).normal();
  Vector p;
  for (double strips = 0.; strips <= num_strips; strips += 1.)
  {
    uint32_t offset = (uint32_t)numVBOElements;
    p = std::cos(2. * M_PI * strips / num_strips) * u +
      std::sin(2. * M_PI * strips / num_strips) * crx;
    p.normalize();
    points.push_back(r1 * p + Vector(p1));
    colors.push_back(color1);
    numVBOElements++;
    points.push_back(r2 * p + Vector(p2));
    colors.push_back(color2);
    numVBOElements++;
    normals.push_back(p);
    normals.push_back(p);
    indices.push_back(0 + offset);
    indices.push_back(1 + offset);
    indices.push_back(2 + offset);
    indices.push_back(2 + offset);
    indices.push_back(1 + offset);
    indices.push_back(3 + offset);
  }
  for (int jj = 0; jj < 6; jj++) indices.pop_back();
}

void GlyphGeom::generateSphere(const Point& center, double radius1, double radius2,
  double resolution, const ColorRGB& color, int64_t& numVBOElements, std::vector<Vector>& points, 
  std::vector<Vector>& normals, std::vector<uint32_t>& indices, std::vector<ColorRGB>& colors)
{
  double num_strips = resolution;
  if (num_strips < 0) num_strips = 20.0;
  double r1 = radius1 < 0 ? 1.0 : radius1;
  double r2 = radius2 < 0 ? 1.0 : radius2;
  Vector pp1, pp2;
  double theta_inc = 2. * M_PI / num_strips, phi_inc = M_PI / num_strips;

  //generate triangles for the spheres
  for (double phi = 0.; phi <= M_PI; phi += phi_inc)
  {
    for (double theta = 0.; theta <= 2. * M_PI; theta += theta_inc)
    {
      uint32_t offset = (uint32_t)numVBOElements;
      pp1 = Vector(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
      pp2 = Vector(sin(theta) * cos(phi + phi_inc), sin(theta) * sin(phi + phi_inc), cos(theta));
      points.push_back(r1 * pp1 + Vector(center));
      colors.push_back(color);
      numVBOElements++;
      points.push_back(r2 * pp2 + Vector(center));
      colors.push_back(color);
      numVBOElements++;
      normals.push_back(pp1);
      normals.push_back(pp2);
      indices.push_back(0 + offset);
      indices.push_back(1 + offset);
      indices.push_back(2 + offset);
      indices.push_back(2 + offset);
      indices.push_back(1 + offset);
      indices.push_back(3 + offset);
    }
    for (int jj = 0; jj < 6; jj++) indices.pop_back();
  }
}

void GlyphGeom::generateEllipsoid(const Point& center, double radius1, double radius2,
  double resolution, const ColorRGB& color, int64_t& numVBOElements, std::vector<Vector>& points,
  std::vector<Vector>& normals, std::vector<uint32_t>& indices, std::vector<ColorRGB>& colors)
{
  double num_strips = resolution;
  if (num_strips < 0) num_strips = 20.0;
  double r1 = radius1 < 0 ? 1.0 : radius1;
  double r2 = radius2 < 0 ? 1.0 : radius2;
  Vector pp1, pp2;
  double theta_inc = /*2. */ M_PI / num_strips, phi_inc = 0.5 * M_PI / num_strips;
  Vector radius = Vector(radius1, 0, radius2);

  //generate triangles for the spheres
  for (double phi = 0.; phi <= M_PI; phi += phi_inc)
  {
    for (double theta = 0.; theta <= /*2. */ M_PI; theta += theta_inc)
    {
      uint32_t offset = (uint32_t)numVBOElements;
      pp1 = Vector(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
      pp2 = Vector(sin(theta) * cos(phi + phi_inc), sin(theta) * sin(phi + phi_inc), cos(theta));
      points.push_back(r1 * pp1 + Vector(center));
      colors.push_back(color);
      numVBOElements++;
      points.push_back(r2 * pp2 + Vector(center + radius));
      colors.push_back(color);
      numVBOElements++;
      normals.push_back(pp1);
      normals.push_back(pp2);

      indices.push_back(0 + offset);
      indices.push_back(1 + offset);
      indices.push_back(2 + offset);
      indices.push_back(2 + offset);
      indices.push_back(1 + offset);
      indices.push_back(3 + offset);
      /*
      indices.push_back(0 + offset);
      indices.push_back(1 + offset);
      indices.push_back(2 + offset);
      indices.push_back(2 + offset);
      indices.push_back(1 + offset);
      indices.push_back(3 + offset);


      //generate triangles for the cylinders.
      Vector n((p1 - p2).normal()), u = (10 * n + Vector(10, 10, 10)).normal();
      Vector crx = Cross(u, n).normal();
      u = Cross(crx, n).normal();
      Vector p;
      for (double strips = 0.; strips <= num_strips; strips += 1.)
      {
      uint32_t offset = (uint32_t)numVBOElements;
      p = std::cos(2. * M_PI * strips / num_strips) * u +
      std::sin(2. * M_PI * strips / num_strips) * crx;
      p.normalize();
      points.push_back(r1 * p + Vector(p1));
      colors.push_back(color1);
      numVBOElements++;
      points.push_back(r2 * p + Vector(p2));
      colors.push_back(color2);
      numVBOElements++;
      normals.push_back(p);
      normals.push_back(p);
      indices.push_back(0 + offset);
      indices.push_back(1 + offset);
      indices.push_back(2 + offset);
      indices.push_back(2 + offset);
      indices.push_back(1 + offset);
      indices.push_back(3 + offset);
      }
      */
    }
    for (int jj = 0; jj < 6; jj++) indices.pop_back();
  }
}

void GlyphGeom::generateLine(const Point p1, const Point& p2, const ColorRGB& color1, const ColorRGB& color2,
  int64_t& numVBOElements, std::vector<Vector>& points, std::vector<uint32_t>& indices, std::vector<ColorRGB>& colors)
{
  points.push_back(Vector(p1));
  colors.push_back(color1);
  indices.push_back(lineIndex_);
  ++lineIndex_;
  points.push_back(Vector(p2));
  colors.push_back(color2);
  indices.push_back(lineIndex_);
  ++lineIndex_;
  ++numVBOElements;
}

void GlyphGeom::generatePoint(const Point p, const ColorRGB& color,
  int64_t& numVBOElements, std::vector<Vector>& points, std::vector<uint32_t>& indices, std::vector<ColorRGB>& colors)
{
  points.push_back(Vector(p));
  colors.push_back(color);
  indices.push_back(lineIndex_);
  ++lineIndex_;
  ++numVBOElements;
}

void GlyphGeom::generatePlane(const Core::Geometry::Point p1, const Core::Geometry::Point p2,
  const Core::Geometry::Point p3, const Core::Geometry::Point p4, const Core::Datatypes::ColorRGB& color,
  int64_t& numVBOElements, std::vector<Core::Geometry::Vector>& points, std::vector<Core::Geometry::Vector>& normals,
  std::vector<uint32_t>& indices, std::vector<Core::Datatypes::ColorRGB>& colors)
{
  points.push_back(Vector(p1));
  points.push_back(Vector(p2));
  points.push_back(Vector(p3));
  points.push_back(Vector(p4));
  colors.push_back(color);
  colors.push_back(color);
  colors.push_back(color);
  colors.push_back(color);
  Vector n;
  n = Cross(p2 - p1, p4 - p1).normal();
  normals.push_back(n);
  n = Cross(p3 - p2, p1 - p2).normal();
  normals.push_back(n);
  n = Cross(p4 - p3, p2 - p3).normal();
  normals.push_back(n);
  n = Cross(p1 - p4, p3 - p4).normal();
  normals.push_back(n);
  indices.push_back(0 + numVBOElements);
  indices.push_back(1 + numVBOElements);
  indices.push_back(2 + numVBOElements);
  indices.push_back(2 + numVBOElements);
  indices.push_back(3 + numVBOElements);
  indices.push_back(0 + numVBOElements);
  numVBOElements += 4;
}

// Addarrow from SCIRun 4
void GlyphGeom::addArrow(const Point& center, const Vector& t,
                         double radius, double length, int nu, int nv)
{
  std::vector<QuadStrip> quadstrips;
  double ratio = 2.0;
  Transform trans;
  Transform rotate;
  generateTransforms(center, t, trans, rotate);
  
  Vector offset = rotate * Vector(0,0,1);
  offset.safe_normalize();
  offset *= length * ratio;
  
  generateCylinder(center, t, radius/10.0, radius/10.0, length*ratio, nu, nv, quadstrips);
  generateCylinder(center+offset, t, radius, 0.0, length, nu, nv, quadstrips);
  
  // add strips to the object
}

// from SCIRun 4
void GlyphGeom::addBox(const Point& center, const Vector& t,
                       double x_side, double y_side, double z_side)
{
  std::vector<QuadStrip> quadstrips;
  generateBox(center, t, x_side, y_side, z_side, quadstrips);
  
  // add strips to object
}

// from SCIRun 4
void GlyphGeom::addCylinder(const Point& center, const Vector& t,
                            double radius1, double length, int nu, int nv)
{
  std::vector<QuadStrip> quadstrips;
  generateCylinder(center, t, radius1, radius1, length, nu, nv, quadstrips);
  
  // add the strips to the object
}

// from SCIRun 4
void GlyphGeom::addSphere(const Point& center, double radius,
                          int nu, int nv, int half)
{
  std::vector<QuadStrip> quadstrips;
  generateEllipsoid(center, Vector(0, 0, 1), radius, nu, nv, half, quadstrips);
  
  // add strips to the object

}

// Generate cylinder from SCIRun 4
void GlyphGeom::generateCylinder(const Point& center, const Vector& t, double radius1,
                                 double radius2, double length, int nu, int nv,
                                 std::vector<QuadStrip>& quadstrips)
{
  nu++; //Bring nu to expected value for shape.

  if (nu > 20) nu = 20;
  if (nv == 0) nv = 20;
  SinCosTable& tab1 = tables_[nu];

  Transform trans;
  Transform rotate;
  generateTransforms(center, t, trans, rotate);

  // Draw the cylinder
  double dz = length / (float)nv;
  double dr = (radius2 - radius1) / (float)nv;

  for (int v = 0; v<nv; v++)
  {
    double z1 = dz * (float)v;
    double z2 = z1 + dz;

    double r1 = radius1 + dr * (float)v;
    double r2 = r1 + dr;

    QuadStrip quadstrip;

    for (int u = 0; u<nu; u++)
    {
      double nx = tab1.sin(u);
      double ny = tab1.cos(u);

      double x1 = r1 * nx;
      double y1 = r1 * ny;

      double x2 = r2 * nx;
      double y2 = r2 * ny;

      double nx1 = length * nx;
      double ny1 = length * ny;

      Point p1 = trans * Point(x1, y1, z1);
      Point p2 = trans * Point(x2, y2, z2);

      Vector v1 = rotate * Vector(nx1, ny1, -dr);
      v1.safe_normalize();

      quadstrip.push_back(std::make_pair(p1, v1));
      quadstrip.push_back(std::make_pair(p2, v1));
    }

    quadstrips.push_back(quadstrip);
  }

}

// generate ellipsoid from SCIRun 4
void GlyphGeom::generateEllipsoid(const Point& center, const Vector& t, double scales,
                                  int nu, int nv, int half, std::vector<QuadStrip>& quadstrips)
{
  nu++; //Bring nu to expected value for shape.

  double start = 0, stop =  M_PI;

  // Half ellipsoid criteria.
  if (half == -1) start = M_PI / 2.0;
  if (half == 1) stop = M_PI / 2.0;
  if (half != 0) nv /= 2;

  // Should only happen when doing half ellipsoids.
  if (nv < 2) nv = 2;

  SinCosTable tab1(nu, 0, 2 * M_PI);
  SinCosTable tab2(nv, start, stop);

  Transform trans;
  Transform rotate;
  generateTransforms(center, t, trans, rotate);

  trans.post_scale(Vector(1.0, 1.0, 1.0) * scales);
  rotate.post_scale(Vector(1.0, 1.0, 1.0) / scales);

  // Draw the ellipsoid
  for (int v = 0; v<nv - 1; v++)
  {
    double nr1 = tab2.sin(v + 1);
    double nr2 = tab2.sin(v);

    double nz1 = tab2.cos(v + 1);
    double nz2 = tab2.cos(v);

    QuadStrip quadstrip;

    for (int u = 0; u<nu; u++)
    {
      double nx = tab1.sin(u);
      double ny = tab1.cos(u);

      double x1 = nr1 * nx;
      double y1 = nr1 * ny;
      double z1 = nz1;

      double x2 = nr2 * nx;
      double y2 = nr2 * ny;
      double z2 = nz2;

      Point p1 = trans * Point(x1, y1, z1);
      Point p2 = trans * Point(x2, y2, z2);

      Vector v1 = rotate * Vector(x1, y1, z1);
      Vector v2 = rotate * Vector(x2, y2, z2);

      v1.safe_normalize();
      v2.safe_normalize();

      quadstrip.push_back(std::make_pair(p1, v1));
      quadstrip.push_back(std::make_pair(p2, v2));
    }

    quadstrips.push_back(quadstrip);
  }
}

//generate box from SCIRun 4
void GlyphGeom::generateBox(const Point& center, const Vector& t, double x_side, double y_side,
                            double z_side, std::vector<QuadStrip>& quadstrips)
{
  double half_x_side = x_side * 0.5;
  double half_y_side = y_side * 0.5;
  double half_z_side = z_side * 0.5;

  Transform trans;
  Transform rotate;
  generateTransforms(center, t, trans, rotate);

  //Draw the Box
  Point p1 = trans * Point(-half_x_side, half_y_side, half_z_side);
  Point p2 = trans * Point(-half_x_side, half_y_side, -half_z_side);
  Point p3 = trans * Point(half_x_side, half_y_side, half_z_side);
  Point p4 = trans * Point(half_x_side, half_y_side, -half_z_side);

  Point p5 = trans * Point(-half_x_side, -half_y_side, half_z_side);
  Point p6 = trans * Point(-half_x_side, -half_y_side, -half_z_side);
  Point p7 = trans * Point(half_x_side, -half_y_side, half_z_side);
  Point p8 = trans * Point(half_x_side, -half_y_side, -half_z_side);

  Vector v1 = rotate * Vector(half_x_side, 0, 0);
  Vector v2 = rotate * Vector(0, half_y_side, 0);
  Vector v3 = rotate * Vector(0, 0, half_z_side);

  Vector v4 = rotate * Vector(-half_x_side, 0, 0);
  Vector v5 = rotate * Vector(0, -half_y_side, 0);
  Vector v6 = rotate * Vector(0, 0, -half_z_side);

  QuadStrip quadstrip1;
  QuadStrip quadstrip2;
  QuadStrip quadstrip3;
  QuadStrip quadstrip4;
  QuadStrip quadstrip5;
  QuadStrip quadstrip6;

  // +X
  quadstrip1.push_back(std::make_pair(p7, v1));
  quadstrip1.push_back(std::make_pair(p8, v1));
  quadstrip1.push_back(std::make_pair(p3, v1));
  quadstrip1.push_back(std::make_pair(p4, v1));

  // +Y
  quadstrip2.push_back(std::make_pair(p3, v2));
  quadstrip2.push_back(std::make_pair(p4, v2));
  quadstrip2.push_back(std::make_pair(p1, v2));
  quadstrip2.push_back(std::make_pair(p2, v2));

  // +Z
  quadstrip3.push_back(std::make_pair(p5, v3));
  quadstrip3.push_back(std::make_pair(p7, v3));
  quadstrip3.push_back(std::make_pair(p1, v3));
  quadstrip3.push_back(std::make_pair(p3, v3));

  // -X
  quadstrip4.push_back(std::make_pair(p1, v4));
  quadstrip4.push_back(std::make_pair(p2, v4));
  quadstrip4.push_back(std::make_pair(p5, v4));
  quadstrip4.push_back(std::make_pair(p6, v4));

  // -Y
  quadstrip5.push_back(std::make_pair(p5, v5));
  quadstrip5.push_back(std::make_pair(p6, v5));
  quadstrip5.push_back(std::make_pair(p7, v5));
  quadstrip5.push_back(std::make_pair(p8, v5));

  // -Z
  quadstrip6.push_back(std::make_pair(p2, v6));
  quadstrip6.push_back(std::make_pair(p4, v6));
  quadstrip6.push_back(std::make_pair(p6, v6));
  quadstrip6.push_back(std::make_pair(p8, v6));

  quadstrips.push_back(quadstrip1);
  quadstrips.push_back(quadstrip2);
  quadstrips.push_back(quadstrip3);
  quadstrips.push_back(quadstrip4);
  quadstrips.push_back(quadstrip5);
  quadstrips.push_back(quadstrip6);
}

// from SCIRun 4
void GlyphGeom::generateTransforms(const Point& center, const Vector& normal,
                                   Transform& trans, Transform& rotate)
{
  Vector axis = normal;

  axis.normalize();

  Vector z(0, 0, 1), zrotaxis;
  
  if((Abs(axis.x()) + Abs(axis.y())) < 1.e-5)
  {
    // Only x-z plane...
    zrotaxis = Vector(0, 1, 0);
  }
  else
  {
    zrotaxis = Cross(axis, z);
    zrotaxis.normalize();
  }
  
  double cangle = Dot(z, axis);
  double zrotangle = -acos(cangle);

  rotate.post_rotate(zrotangle, zrotaxis);
}
