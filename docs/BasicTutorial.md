---
title: Basic Tutorial
category: info
tags: tutorial
layout: default_toc
---

<script type="text/javascript" async
  src="https://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS_CHTML">
</script>
<link rel="stylesheet" href="css/main.css">


<!-- ### Contents
  - [1 SCIRun Overview](#1--scirun-overview)  
    + [1.1 Software requirements](#11--software-requirements)   
  - [2 Simple Dataflow Network](#2--simple-dataflow-network)  
    + [2.1 Slice Field](#21--slice-field)  
      - [2.1.1 Read Data File](#211--read-data-file)  
      - [2.1.2 Slice Field](#212--slice-field)  
      - [2.1.3 Visualize Field](#213--visualize-field)  
    + [2.2 Show Bounding Box](#22--show-bounding-box)  
    + [2.3 Isosurface](#23--isosurface)    
  - [3 Create, Manipulate and Visualize Field](#3--create-manipulate-and-visualize-field)  
    + [3.1 Create Field](#31--create-field)  
    + [3.2 Isosurface](#32--isosurface)  
    + [3.3 Slice Field](#33--slice-field)  
    + [3.4 Clip Field](#34--clip-field)  
      - [3.4.1 Extract Boundary](#341--extract-boundary)   -->

## 1  SCIRun Overview

This tutorial demonstrates how to build a simple SCIRun dataflow network.

### 1.1  Software requirements

#### 1.1.1  SCIRun

All available downloads for SCIRun version and the SCIRunData archive are available from [SCI software portal](https://github.com/SCIInstitute/SCIRun). Make sure to update to the most up-to-date release available, which will include the latest bug fixes.

Currently, the easiest way to get started with SCIRun version is to download and install a binary version for Mac OS X. Sources are also available for Linux, however this option is recommended only for advanced Linux users.

Unpack the SCIRunData archive in a convenient location. Recall from the User Guide that the path to data can be set using the environment variable or by setting in the *.scirunrc* file.

## 2  Simple Dataflow Network
**Scope: [Read Data File](#211--read-data-file) - [Slice](#212--slice-field) - [Visualize](#213--visualize-field) - [Bounding Box](#22--show-bounding-box) - [Isosurface](#23--isosurface)**

### 2.1  Slice Field

The purpose of this section is to read, manipulate, and visualize a structured mesh dataset originating from SCIRunData.

#### 2.1.1  Read Data File

Create a **ReadField** module by using the **Module Selector** on the left hand side of the screen. Navigate to **DataIO** subsection using the scroll bar in the Module Selector and instantiate a ReadField (<a href="#readfield">Figure 2.1</a>). Recall from the **User Guide** that a module can also be selected by giving a text input into the filter in the Module Selector (<a href="#readfield_text">Figure 2.2</a>).

<figure id="readfield">
  <img src="BasicTutorial_figures/readfield.png" alt="Locate ReadField module using scroll bar in the Module Selector.">
  <figcaption>Figure 2.1 Locate ReadField module using scroll bar in the Module Selector.</figcaption>
</figure>

<figure id="readfield_text">
  <img src="BasicTutorial_figures/readfield_text.png" alt="Locate ReadField module using text input into filter."/>
  <figcaption>Figure 2.2 Locate ReadField module using text input into filter.</figcaption>
</figure>

Within the ReadField **user interface (UI)**, click the open button to navigate to the SCIRunData directory and select the dataset *volume/engine.nhdr* (<a href="#readfield_type">Figure 2.3</a>). Notice that many different file formats can be imported by changing the file type within the ReadField selector window. When using Mac OSX El Capitan, press the options button in the ReadField selector window to change the file type. Change the file type to Nrrd file. The ReadField UI can be closed after selection to provide for a larger network viewing frame.

<figure id="readfield_type">
  <img src="BasicTutorial_figures/readfield_select.png" alt="The ReadField selector window can be used to select and read many data files."/>
  <figcaption>Figure 2.3 The ReadField selector window can be used to select and read many data files.</figcaption>
</figure>

#### 2.1.2  Slice Field

Slice the engine field by node index along a given axis by instantiating the module **GetSlicesFromStructuredFieldByIndices** in the **NewField** category and connecting it to ReadField (<a href="#slicemod">Figure 2.4</a>). This can be done by using the Module Selector filter or scrolling through the list of modules in the Module Selector.

<figure id="slicemod">
  <img src="BasicTutorial_figures/getslice.png" alt="Using the ReadField port’s pop-up module menu to instantiate GetSliceFromStructuredFieldByIndices."/>
  <figcaption>Figure 2.4 Using the ReadField port’s pop-up module menu to instantiate GetSliceFromStructuredFieldByIndices.</figcaption>
</figure>

#### 2.1.3  Visualize Field

To visualize the field geometry, instantiate module **ShowField** in the **Visualization** category and module **ViewScene** in the **Render** category (<a href="#colormap">Figure 2.5</a>). ShowField takes a field as input, and outputs scene-graph geometry. ViewScene displays the geometry and allows a user to interact with the scene.

<figure id="colormap">
  <img src="BasicTutorial_figures/viewscene.png" alt="SCIRun can be used to visualize the structured mesh."/>
  <figcaption>Figure 2.5 SCIRun can be used to visualize the structured mesh.</figcaption>
</figure>

Apply a colored scale to the data values on the geometry using **CreateStandardColorMaps** and **RescaleColorMaps** modules in **Visualization** (<a href="#rescale">Figure 2.6</a>). Colors can be manipulated using the CreateStandardColorMap UI and RescaleColorMap UI (<a href="#viewscene">Figure 2.7</a>). Change the coloring scheme to Blackbody using the drop-down menu in the CreatSrandardColorMap UI.

<figure id="rescale">
  <img src="BasicTutorial_figures/colorscale.png" alt="Apply and rescale a colormap to data values on the geometry."/>
  <figcaption>Figure 2.6 Apply and rescale a colormap to data values on the geometry.</figcaption>
</figure>

<figure id="viewscene">
  <img src="BasicTutorial_figures/manipulatecolorscale.png" alt="Manipulate the color scaling using both the CreateStandardColorMaps and RescaleColorMaps modules."/>
  <figcaption>Figure 2.7 Manipulate the color scaling using both the CreateStandardColorMaps and RescaleColorMaps modules.</figcaption>
</figure>

Return to the default color scale. Use the sliders in the GetSlicesFromStructuredFieldByIndices UI to change slice position within the geometry. Compare with <a href="#rescale">Figure 2.6.</a>

<figure id="getsclice">
  <img src="BasicTutorial_figures/sliceselect.png" alt="Different cross sections can be visualized within the geometry using GetSlicesFromStructuredFieldbyIndices. "/>
  <figcaption>Figure 2.8 Different cross sections can be visualized within the geometry using GetSlicesFromStructuredFieldbyIndices.</figcaption>
</figure>

### 2.2  Show Bounding Box

Add the **EditMeshBoundingBox** module under **ChangeMesh** (<a href="#addbbox">Figure 2.9</a>). Connect it to the ReadField module and direct the output to the ViewScene module. Execute the network to visualize the bounding box of engine.nhrd. Adjust the size of the bounding box by pressing the + or - buttons under Widget Scale in the EditMeshBoundingBox UI (<a href="#addbbox2">Figure 2.10</a>).

<figure id="addbbox">
  <img src="BasicTutorial_figures/editmeshboundingbox.png" alt="Visualize the mesh’s bounding box."/>
  <figcaption>Figure 2.9 Visualize the mesh’s bounding box.</figcaption>
</figure>

<figure id="addbbox2">
  <img src="BasicTutorial_figures/editmeshbbox.png" alt="Change the scale of the mesh’s bounding box using the Scale Widget in the EditMeshBoundingBox UI."/>
  <figcaption>Figure 2.10 Change the scale of the mesh’s bounding box using the Scale Widget in the EditMeshBoundingBox UI.</figcaption>
</figure>

### 2.3  Isosurface

Construct an isosurface from the field by instantiating and connecting a **ExtractSimpleIsosurface** module to the ReadField module. The isovalue must be changed within the ExtractSimpleIsosurface UI. Open the field information by clicking on the connection between the ReadField and ExtractSimpleIsosurface and press I to bring up information. Enter a value from within the data range like 120. Visualize the isosurface by connecting it to a new ShowField module ported into the ViewScene module (<a href="#extract">Figure 2.11</a>). Execute the network. Color isosurface output geometry by connecting the RescaleColorMap module to the ShowField module (<a href="#changeisoval">Figure 2.12</a>). To better view the geometry, turn off the edges within the ShowField UI (<a href="#viewisosurf">Figure 2.13</a>).

<figure id="extract">
  <img src="BasicTutorial_figures/extractiso.png" alt="Extract an isosurface from field."/>
  <figcaption>Figure 2.11 Extract an isosurface from field.</figcaption>
</figure>

<figure id="changeisoval">
  <img src="BasicTutorial_figures/coloriso.png" alt="Change the isovalue within ExtractSimpleIsosurface UI."/>
  <figcaption>Figure 2.12 Change the isovalue within ExtractSimpleIsosurface UI.</figcaption>
</figure>

<figure id="viewisosurf">
  <img src="BasicTutorial_figures/edgesiso.png" alt="Adjusting parameters within the ShowField UI helps to better visualize the isosurface."/>
  <figcaption>Figure 2.13 Adjusting parameters within the ShowField UI helps to better visualize the isosurface.</figcaption>
</figure>

## 3  Create, Manipulate and Visualize Field
**Scope: [Generate Lattice Volume](#31--create-field) [Isosurface](#32--isosurface) Visualize Geometry**

### 3.1  Create Field

Create and manipulate a structured mesh type in this exercise. Start by creating a lattice volume using **CreateLatVol** module. Assign data at nodes using **CalculateFieldData** module. Connect CalculateFieldData to CreateLatVol. Input the expression $$RESULT = sqrt(X * X + Y * Y + Z * Z)$$ to compute data for each node within the CreateFieldData UI.

<figure id="createnewfield">
  <img src="BasicTutorial_figures/create.png" alt="Create lattice volume field using CreateLatVol module."/>
  <figcaption>Figure 3.1 Create lattice volume field using CreateLatVol module.</figcaption>
</figure>

<figure id="calcfielddata">
  <img src="BasicTutorial_figures/fielddata.png" alt="Create a new field by inputting an expression into the CreateFieldData UI. "/>
  <figcaption>Figure 3.2 Create a new field by inputting an expression into the CreateFieldData UI.</figcaption>
</figure>

### 3.2  Isosurface

Generate the isosurface by instantiating and connecting an ExtractSimpleIsosurface module to CalculateFieldData (<a href="#extractisosurf2">Figure 3.3</a>). Adjust the isovalue within the ExtractSimpleIsosurface UI so that the isosurface can be visualized (<a href="#conncolormap">Figure 3.4</a>). Add a color map and visualize the isosurface as in [section 2.3](#23--isosurface) (<a href="#viewdefaultisosurf">Figure 3.5</a>). Show the mesh bounding box as in [section 2.2](#22--show-bounding-box) (<a href="#changeisoval2">Figure 3.6</a>).

<figure id="extractisosurf2">
  <img src="BasicTutorial_figures/extractiso2.png" alt="Extract an
  from the field data."/>
  <figcaption>Figure 3.3 Extract an isosurface from the field data.</figcaption>
</figure>

<figure id="conncolormap">
  <img src="BasicTutorial_figures/selectisoval.png" alt="Change the isovalue so that an isosurface can be visualized."/>
  <figcaption>Figure 3.4 Change the isovalue so that an isosurface can be visualized.</figcaption>
</figure>

<figure id="viewdefaultisosurf">
  <img src="BasicTutorial_figures/viewisocirc.png" alt="Visualize the isosurface."/>
  <figcaption>Figure 3.5 Visualize the isosurface.</figcaption>
</figure>

<figure id="changeisoval2">
  <img src="BasicTutorial_figures/bbox.png" alt="Visualize the mesh’s bounding box."/>
  <figcaption>Figure 3.6 Visualize the mesh’s bounding box.</figcaption>
</figure>

### 3.3  Slice Field

Extend the functionality of this network by slicing the field using GetSliceFromStructuredFieldByIndices as in [section 2.1.2](#212--slice-field).

<figure id="slicemod2">
  <img src="BasicTutorial_figures/getslice2.png" alt="Insert GetSliceFromStructuredFieldByIndices into the network."/>
  <figcaption>Figure 3.7 Insert GetSliceFromStructuredFieldByIndices into the network.</figcaption>
</figure>

<figure id="showslice">
  <img src="BasicTutorial_figures/changeslice.png" alt="Change the slice index using the GetSliceFromStructuredFieldByIndices UI."/>
  <figcaption>Figure 3.8 Change the slice index using the GetSliceFromStructuredFieldByIndices UI.</figcaption>
</figure>

<figure id="showsslice">
  <img src="BasicTutorial_figures/colorslice.png" alt="Attach the RescaleColorMap module to the ShowField module."/>
  <figcaption>Figure 3.9 Attach the RescaleColorMap module to the ShowField module.</figcaption>
</figure>

### 3.4  Clip Field

Clip out a subset of the original field by converting the lattice volume to an unstructured mesh using **ConvertMeshToUnstructuredMesh** (<a href="#clipfieldfunc">Figure 3.10</a>) and adding **ClipFieldByFunction** (<a href="#convertmesh">Figure 3.11</a>) to the network. Set the clipping location setting in ClipFieldByFunction to *all nodes*. Use the expression _DATA1 &gt; 1&&X &lt; 0_ to clip the field (<a href="#clipfield">Figure 3.12</a>).

<figure id="clipfieldfunc">
  <img src="BasicTutorial_figures/convertmesh.png" alt="Convert the original field to an unstructured mesh."/>
  <figcaption>Figure 3.10 Convert the original field to an unstructured mesh.</figcaption>
</figure>

<figure id="convertmesh">
  <img src="BasicTutorial_figures/clipfield.png" alt="Insert a ClipFieldbyFunction module."/>
  <figcaption>Figure 3.11 Insert a ClipFieldbyFunction module.</figcaption>
</figure>

<figure id="clipfield">
  <img src="BasicTutorial_figures/clipfield_input.png" alt="Clip the field by entering an expression in the ClipField UI."/>
  <figcaption>Figure 3.12 Clip the field by entering an expression in the ClipField UI.</figcaption>
</figure>

#### 3.4.1  Extract Boundary

At this point, it will be necessary to map the fields by interpolating the the boundary surface field to the clipping field. First, use **BuildMappingMatrix** to build a matrix that maps a linear combination of data values in the clipping field to a value in the boundary field. Then use **ApplyMappingMatrix** to multiply the data vector of the clipping field with the mapping matrix to obtain the data vector for the boundary surface field (<a href="#clipfieldinput">Figure 3.13</a>). Use GetFieldBoundary to extract the boundary surface from the lattice volume and use it as input into the ApplyMappingMatrixModule and BuildMapping Matrix (<a href="#mappingmatrix">Figure 3.14</a>). Port the output from the BuildMappingMatrix module to ApplyMappingMatrix and visualize the resultant field using a ShowFieldModule (<a href="#highlightmods">Figure 3.15</a>). Add a colormap to and enable transparency in ShowField UI for further functionality (<a href="#viewbound">Figure 3.16</a>)

<figure id="clipfieldinput">
  <img src="BasicTutorial_figures/mappingmatrix.png" alt="Build and apply the mapping network connections."/>
  <figcaption>Figure 3.13 Build and apply the mapping network connections.</figcaption>
</figure>

<figure id="mappingmatrix">
  <img src="BasicTutorial_figures/fieldboundary.png" alt="Add GetFieldBoundary to the network."/>
  <figcaption>Figure 3.14 Add GetFieldBoundary to the network.</figcaption>
</figure>

<figure id="highlightmods">
  <img src="BasicTutorial_figures/view_boundary.png" alt="Connect all the modules for mapping and visualize the output."/>
  <figcaption>Figure 3.15 Connect all the modules for mapping and visualize the output.</figcaption>
</figure>

<figure id="viewbound">
  <img src="BasicTutorial_figures/finalview.png" alt="Add a colormap and enable transparency."/>
  <figcaption>Figure 3.16 Add a colormap and enable transparency.</figcaption>
</figure>

Finally, it is not strictly necessary to explicitly convert the original mesh to an unstructured mesh using ConvertMeshToUnstructuredMesh because ClipFieldByFunction can implicitly convert structured mesh types to unstructured mesh types before clipping the field. As a final exercise, delete ConvertMeshToUnstructuredMesh from the network and try to obtain the same result.
