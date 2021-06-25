# Basic Tutorial
<script type="text/javascript" async
  src="https://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS_CHTML">
</script>

## SCIRun Overview

This tutorial demonstrates how to build a simple SCIRun dataflow network.

### Software requirements

#### SCIRun

All available downloads for SCIRun version and the SCIRunData archive are available from [SCI software portal](https://github.com/SCIInstitute/SCIRun). Make sure to update to the most up-to-date release available, which will include the latest bug fixes.

Currently, the easiest way to get started with SCIRun version is to download and install a binary version for Mac OS X. Sources are also available for Linux, however this option is recommended only for advanced Linux users.

Unpack the SCIRunData archive in a convenient location. Recall from the User Guide that the path to data can be set using the environment variable or by setting in the *.scirunrc* file.

## Simple Dataflow Network

### Slice Field

The purpose of this section is to read, manipulate, and visualize a structured mesh dataset originating from SCIRunData.

#### Read Data File

Create a **ReadField** module by using the **Module Selector** on the left hand side of the screen. Navigate to **DataIO** subsection using the scroll bar in the Module Selector and instantiate a ReadField (<a href="#readfield">Figure 2.1</a>). Recall from the **User Guide** that a module can also be selected by giving a text input into the filter in the Module Selector (<a href="#readfield_text">Figure 2.2</a>).

<!-- readfield -->
![Locate ReadField module using scroll bar in the Module Selector.](BasicTutorial_figures/readfield.png)
<figcaption>Figure 2.1 Locate ReadField module using scroll bar in the Module Selector.</figcaption>


<!-- readfield_text -->
![Locate ReadField module using text input into filter.](BasicTutorial_figures/readfield_text.png)
<figcaption>Figure 2.2 Locate ReadField module using text input into filter.</figcaption>


Within the ReadField **user interface (UI)**, click the open button to navigate to the SCIRunData directory and select the dataset *volume/engine.nhdr* (<a href="#readfield_type">Figure 2.3</a>). Notice that many different file formats can be imported by changing the file type within the ReadField selector window. When using Mac OSX El Capitan, press the options button in the ReadField selector window to change the file type. Change the file type to Nrrd file. The ReadField UI can be closed after selection to provide for a larger network viewing frame.

<!-- readfield_type -->
![The ReadField selector window can be used to select and read many data files.](BasicTutorial_figures/readfield_select.png)
<figcaption>Figure 2.3 The ReadField selector window can be used to select and read many data files.</figcaption>


#### Slice Field

Slice the engine field by node index along a given axis by instantiating the module **GetSlicesFromStructuredFieldByIndices** in the **NewField** category and connecting it to ReadField (<a href="#slicemod">Figure 2.4</a>). This can be done by using the Module Selector filter or scrolling through the list of modules in the Module Selector.

<!-- slicemod -->
![Using the ReadField port’s pop-up module menu to instantiate GetSliceFromStructuredFieldByIndices.](BasicTutorial_figures/getslice.png)
<figcaption>Figure 2.4 Using the ReadField port’s pop-up module menu to instantiate GetSliceFromStructuredFieldByIndices.</figcaption>


#### Visualize Field

To visualize the field geometry, instantiate module **ShowField** in the **Visualization** category and module **ViewScene** in the **Render** category (<a href="#colormap">Figure 2.5</a>). ShowField takes a field as input, and outputs scene-graph geometry. ViewScene displays the geometry and allows a user to interact with the scene.

<!-- colormap -->
![SCIRun can be used to visualize the structured mesh.](BasicTutorial_figures/viewscene.png)
<figcaption>Figure 2.5 SCIRun can be used to visualize the structured mesh.</figcaption>


Apply a colored scale to the data values on the geometry using **CreateStandardColorMaps** and **RescaleColorMaps** modules in **Visualization** (<a href="#rescale">Figure 2.6</a>). Colors can be manipulated using the CreateStandardColorMap UI and RescaleColorMap UI (<a href="#viewscene">Figure 2.7</a>). Change the coloring scheme to Blackbody using the drop-down menu in the CreatSrandardColorMap UI.

<!-- rescale -->
![Apply and rescale a colormap to data values on the geometry.](BasicTutorial_figures/colorscale.png)
<figcaption>Figure 2.6 Apply and rescale a colormap to data values on the geometry.</figcaption>


<!-- viewscene -->
![Manipulate the color scaling using both the CreateStandardColorMaps and RescaleColorMaps modules.](BasicTutorial_figures/manipulatecolorscale.png)
<figcaption>Figure 2.7 Manipulate the color scaling using both the CreateStandardColorMaps and RescaleColorMaps modules.</figcaption>


Return to the default color scale. Use the sliders in the GetSlicesFromStructuredFieldByIndices UI to change slice position within the geometry. Compare with <a href="#rescale">Figure 2.6.</a>

<!-- getslice -->
![Different cross sections can be visualized within the geometry using GetSlicesFromStructuredFieldbyIndices.](BasicTutorial_figures/sliceselect.png)
<figcaption>Figure 2.8 Different cross sections can be visualized within the geometry using GetSlicesFromStructuredFieldbyIndices.</figcaption>


### Show Bounding Box

Add the **EditMeshBoundingBox** module under **ChangeMesh** (<a href="#addbbox">Figure 2.9</a>). Connect it to the ReadField module and direct the output to the ViewScene module. Execute the network to visualize the bounding box of engine.nhrd. Adjust the size of the bounding box by pressing the + or - buttons under Widget Scale in the EditMeshBoundingBox UI (<a href="#addbbox2">Figure 2.10</a>).

<!-- addbbox -->
![Visualize the mesh’s bounding box.](BasicTutorial_figures/editmeshboundingbox.png)
<figcaption>Figure 2.9 Visualize the mesh’s bounding box.</figcaption>


<!-- addbbox2 -->
![Change the scale of the mesh’s bounding box using the Scale Widget in the EditMeshBoundingBox UI.](BasicTutorial_figures/editmeshbbox.png)
<figcaption>Figure 2.10 Change the scale of the mesh’s bounding box using the Scale Widget in the EditMeshBoundingBox UI.</figcaption>


### Isosurface

Construct an isosurface from the field by instantiating and connecting a **ExtractSimpleIsosurface** module to the ReadField module. The isovalue must be changed within the ExtractSimpleIsosurface UI. Open the field information by clicking on the connection between the ReadField and ExtractSimpleIsosurface and press I to bring up information. Enter a value from within the data range like 120. Visualize the isosurface by connecting it to a new ShowField module ported into the ViewScene module (<a href="#extract">Figure 2.11</a>). Execute the network. Color isosurface output geometry by connecting the RescaleColorMap module to the ShowField module (<a href="#changeisoval">Figure 2.12</a>). To better view the geometry, turn off the edges within the ShowField UI (<a href="#viewisosurf">Figure 2.13</a>).

<!-- extract -->
![Extract an isosurface from field.](BasicTutorial_figures/extractiso.png)
<figcaption>Figure 2.11 Extract an isosurface from field.</figcaption>


<!-- changeisoval -->
![Change the isovalue within ExtractSimpleIsosurface UI.](BasicTutorial_figures/coloriso.png)
<figcaption>Figure 2.12 Change the isovalue within ExtractSimpleIsosurface UI.</figcaption>


<!-- viewisosurf -->
![Adjusting parameters within the ShowField UI helps to better visualize the isosurface.](BasicTutorial_figures/edgesiso.png)
<figcaption>Figure 2.13 Adjusting parameters within the ShowField UI helps to better visualize the isosurface.</figcaption>


## Create, Manipulate and Visualize Field

### Create Field

Create and manipulate a structured mesh type in this exercise. Start by creating a lattice volume using **CreateLatVol** module. Assign data at nodes using **CalculateFieldData** module. Connect CalculateFieldData to CreateLatVol. Input the following expression \\( RESULT = sqrt(X * X + Y * Y + Z * Z) \\) to compute data for each node within the CreateFieldData UI.

<!-- createnewfield -->
![Create lattice volume field using CreateLatVol module.](BasicTutorial_figures/create.png)
<figcaption>Figure 3.1 Create lattice volume field using CreateLatVol module.</figcaption>


<!-- calcfielddata -->
![Create a new field by inputting an expression into the CreateFieldData UI.](BasicTutorial_figures/fielddata.png)
<figcaption>Figure 3.2 Create a new field by inputting an expression into the CreateFieldData UI.</figcaption>


### Isosurface

Generate the isosurface by instantiating and connecting an ExtractSimpleIsosurface module to CalculateFieldData (<a href="#extractisosurf2">Figure 3.3</a>). Adjust the isovalue within the ExtractSimpleIsosurface UI so that the isosurface can be visualized (<a href="#conncolormap">Figure 3.4</a>). Add a color map and visualize the isosurface as in [section 2.3](#23--isosurface) (<a href="#viewdefaultisosurf">Figure 3.5</a>). Show the mesh bounding box as in [section 2.2](#22--show-bounding-box) (<a href="#changeisoval2">Figure 3.6</a>).

<!-- extractisosurf2 -->
![Extract an isosurface from the field data.](BasicTutorial_figures/extractiso2.png)
<figcaption>Figure 3.3 Extract an isosurface from the field data.</figcaption>


<!-- conncolormap -->
![Change the isovalue so that an isosurface can be visualized.](BasicTutorial_figures/selectisoval.png)
<figcaption>Figure 3.4 Change the isovalue so that an isosurface can be visualized.</figcaption>


<!-- viewdefaultisosurf -->
![Visualize the isosurface.](BasicTutorial_figures/viewisocirc.png)
<figcaption>Figure 3.5 Visualize the isosurface.</figcaption>


<!-- changeisoval2 -->
![Visualize the mesh’s bounding box.](BasicTutorial_figures/bbox.png)
<figcaption>Figure 3.6 Visualize the mesh’s bounding box.</figcaption>


### Slice Field

Extend the functionality of this network by slicing the field using GetSliceFromStructuredFieldByIndices as in [section 2.1.2](#212--slice-field).

<!-- slicemod2 -->
![Insert GetSliceFromStructuredFieldByIndices into the network.](BasicTutorial_figures/getslice2.png)
<figcaption>Figure 3.7 Insert GetSliceFromStructuredFieldByIndices into the network.</figcaption>


<!-- showslice -->
![Change the slice index using the GetSliceFromStructuredFieldByIndices UI.](BasicTutorial_figures/changeslice.png)
<figcaption>Figure 3.8 Change the slice index using the GetSliceFromStructuredFieldByIndices UI.</figcaption>


<!-- showsslice -->
![Attach the RescaleColorMap module to the ShowField module.](BasicTutorial_figures/colorslice.png)
<figcaption>Figure 3.9 Attach the RescaleColorMap module to the ShowField module.</figcaption>


### Clip Field

Clip out a subset of the original field by converting the lattice volume to an unstructured mesh using **ConvertMeshToUnstructuredMesh** (<a href="#clipfieldfunc">Figure 3.10</a>) and adding **ClipFieldByFunction** (<a href="#convertmesh">Figure 3.11</a>) to the network. Set the clipping location setting in ClipFieldByFunction to *all nodes*. Use the expression `DATA1 > 1 && X < 0` to clip the field (<a href="#clipfield">Figure 3.12</a>).

<!-- clipfieldfunc -->
![Convert the original field to an unstructured mesh.](BasicTutorial_figures/convertmesh.png)
<figcaption>Figure 3.10 Convert the original field to an unstructured mesh.</figcaption>


<!-- convertmesh -->
![Insert a ClipFieldbyFunction module.](BasicTutorial_figures/clipfield.png)
<figcaption>Figure 3.11 Insert a ClipFieldbyFunction module.</figcaption>


<!-- clipfield -->
![Clip the field by entering an expression in the ClipField UI.](BasicTutorial_figures/clipfield_input.png)
<figcaption>Figure 3.12 Clip the field by entering an expression in the ClipField UI.</figcaption>


#### Extract Boundary

At this point, it will be necessary to map the fields by interpolating the the boundary surface field to the clipping field. First, use **BuildMappingMatrix** to build a matrix that maps a linear combination of data values in the clipping field to a value in the boundary field. Then use **ApplyMappingMatrix** to multiply the data vector of the clipping field with the mapping matrix to obtain the data vector for the boundary surface field (<a href="#clipfieldinput">Figure 3.13</a>). Use GetFieldBoundary to extract the boundary surface from the lattice volume and use it as input into the ApplyMappingMatrixModule and BuildMapping Matrix (<a href="#mappingmatrix">Figure 3.14</a>). Port the output from the BuildMappingMatrix module to ApplyMappingMatrix and visualize the resultant field using a ShowFieldModule (<a href="#highlightmods">Figure 3.15</a>). Add a colormap to and enable transparency in ShowField UI for further functionality (<a href="#viewbound">Figure 3.16</a>)

<!-- clipfieldinput -->
![Build and apply the mapping network connections.](BasicTutorial_figures/mappingmatrix.png)
<figcaption>Figure 3.13 Build and apply the mapping network connections.</figcaption>


<!-- mappingmatrix -->
![Add GetFieldBoundary to the network.](BasicTutorial_figures/fieldboundary.png)
<figcaption>Figure 3.14 Add GetFieldBoundary to the network.</figcaption>


<!-- highlightmods -->
![Connect all the modules for mapping and visualize the output.](BasicTutorial_figures/view_boundary.png)
<figcaption>Figure 3.15 Connect all the modules for mapping and visualize the output.</figcaption>


<!-- viewbound -->
![Add a colormap and enable transparency.](BasicTutorial_figures/finalview.png)
<figcaption>Figure 3.16 Add a colormap and enable transparency.</figcaption>


Finally, it is not strictly necessary to explicitly convert the original mesh to an unstructured mesh using ConvertMeshToUnstructuredMesh because ClipFieldByFunction can implicitly convert structured mesh types to unstructured mesh types before clipping the field. As a final exercise, delete ConvertMeshToUnstructuredMesh from the network and try to obtain the same result.
