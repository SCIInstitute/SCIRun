---
title: ExtractSimpleIsosurface
category: moduledocs
module:
  category: NewField
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary
This moudle extracts an isopotential surface from a scalar field.

**Detailed Description**

The ExtractIsosurface module is used to extract one or more isopotential surfaces from a scalar field using the Marching Cubes algorithm. The isopotential surfaces are surfaces for which the scalar value would interpolate to a constant isovalue. The module can output a SCIRun field and geometry (for faster module execution, if only one is needed, deselect the output type that is not needed). The isovalues can be specified in several ways.

#### Slider

The Slider tab allows the user to select one isovalue using a slider bar or to type in one isovalue manually. The slider bar ranges from the minimum and maximum values of the input field. This can be used with great effect to interactively move the isosurface around when the **Update Auto** option is specified. Other update options are **On Release**, which updates the isosurface when the slider button is released, and **Manual**, which updates the isosurface when the module is executed.

#### Quantity

The Quantity tab allows for the selection of several regularly spaced isosurfaces (the list of isovalues is not editable). This is particularly useful for isocontouring as it gives a contour-map effect for where the isosurfaces would be located.

#### List

The List tab allows the user to type in an arbitrary spaced delimited list of isovalues to be used. In addition to hard numbers, percentages relative to the field minimum and maximum may be specified as well. For instance, 50% would specify the isovalue in the center of the min-max range, whereas 50.0 would isosurface at a value of 50.0.

#### Matrix

Isovalues can be passed into the field in the **Optional Isovalues** port, which expects a Nx1 matrix. The Matrix tab must be selected and the module executed to populate the list of isovalues from the **Optional Isovalues** input matrix. A surface will be created for each value in that matrix. The *ExtractIsosurface-probe.srn* example network demonstrates this using the {% include moduleLink.md moduleName='GenerateSinglePointProbeFromField' %} and {% include moduleLink.md moduleName='SwapFieldDataWithMatrixEntries' %} modules to interactively select the isosurface with a probe using this port.


The **Build Output Field** option determines whether or not a surface field is created. The **Build Output Geometry** option determines whether or not geometry data is created. Transparency can be enabled for geometry output by selecting the **Enable Transparency** option. If there is no **ColorMap** present then the default color is used for the output geometry.


This module can work on scalar fields with the following mesh type: LatVol, StructHexVol, HexVol, TetVol, Prism, Image, StructQuadSurf, QuadSurf, TriSurf, Scanline, StructCurve, and Curve. Isosurfaces are generated for volume meshes, isocontours are generated for surface meshes, and isopoints are generated for edge meshes.

If the scalar fields contain cell-centered data, the surfaces are created along the appropriate cell faces. In the case where the isovalue exactly equals the scalar data only the face between the cell and its neighbor with a great scalar value will be extracted.

#### Output

There are four output ports:

  1. The first output port contains the field
  2. The second output port is the geometry (useful for large data sets if the extracted surface will only be viewed)
  3. The third output port contains a mapping matrix to the nodes used to build the isosurface
  4. The fourth output port contains a mapping matrix to the cells used to build the isosurface.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
