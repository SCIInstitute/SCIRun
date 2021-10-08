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

:::{figure-md} fig-read-field

![Locate ReadField module using scroll bar in the Module Selector.](BasicTutorial_figures/readfield.png)

Locate ReadField module using scroll bar in the Module Selector.
:::

Create a **ReadField** module by using the **Module Selector** on the left hand side of the screen. Navigate to **DataIO** subsection using the scroll bar in the Module Selector and instantiate a ReadField ({numref}`fig-read-field`). Recall from the **User Guide** that a module can also be selected by giving a text input into the filter in the Module Selector ({numref}`fig-read-field-text`).

:::{figure-md} fig-read-field-text

![Locate ReadField module using text input into filter.](BasicTutorial_figures/readfield_text.png)

Locate ReadField module using text input into filter.
:::

Within the ReadField **user interface (UI)**, click the open button to navigate to the SCIRunData directory and select the dataset *volume/engine.nhdr* ({numref}`fig-read-field-type`). Notice that many different file formats can be imported by changing the file type within the ReadField selector window. When using Mac OSX El Capitan, press the options button in the ReadField selector window to change the file type. Change the file type to Nrrd file. The ReadField UI can be closed after selection to provide for a larger network viewing frame.

:::{figure-md} fig-read-field-type

![The ReadField selector window can be used to select and read many data files.](BasicTutorial_figures/readfield_select.png)

The ReadField selector window can be used to select and read many data files.
:::

#### Slice Field

Slice the engine field by node index along a given axis by instantiating the module **GetSlicesFromStructuredFieldByIndices** in the **NewField** category and connecting it to ReadField ({numref}`fig-slice-mod`). This can be done by using the Module Selector filter or scrolling through the list of modules in the Module Selector.

:::{figure-md} fig-slice-mod

![Using the ReadField port’s pop-up module menu to instantiate GetSliceFromStructuredFieldByIndices.](BasicTutorial_figures/getslice.png)

Using the ReadField port’s pop-up module menu to instantiate GetSliceFromStructuredFieldByIndices.
:::

#### Visualize Field

:::{figure-md} fig-colormap

![SCIRun can be used to visualize the structured mesh.](BasicTutorial_figures/viewscene.png)

SCIRun can be used to visualize the structured mesh.
:::

To visualize the field geometry, instantiate module **ShowField** in the **Visualization** category and module **ViewScene** in the **Render** category ({numref}`fig-colormap`). ShowField takes a field as input, and outputs scene-graph geometry. ViewScene displays the geometry and allows a user to interact with the scene.

:::{figure-md} fig-rescale

![Apply and rescale a colormap to data values on the geometry.](BasicTutorial_figures/colorscale.png)

Apply and rescale a colormap to data values on the geometry.
:::

Apply a colored scale to the data values on the geometry using **CreateStandardColorMaps** and **RescaleColorMaps** modules in **Visualization** ({numref}`fig-rescale`). Colors can be manipulated using the CreateStandardColorMap UI and RescaleColorMap UI ({numref}`fig-viewscene`). Change the coloring scheme to Blackbody using the drop-down menu in the CreateStandardColorMap UI.

:::{figure-md} fig-viewscene

![Manipulate the color scaling using both the CreateStandardColorMaps and RescaleColorMaps modules.](BasicTutorial_figures/manipulatecolorscale.png)

Manipulate the color scaling using both the CreateStandardColorMaps and RescaleColorMaps modules.
:::

Return to the default color scale. Use the sliders in the GetSlicesFromStructuredFieldByIndices UI to change slice position within the geometry. Compare with {numref}`fig-rescale`.

:::{figure-md} fig-get-slice

![Different cross sections can be visualized within the geometry using GetSlicesFromStructuredFieldbyIndices.](BasicTutorial_figures/sliceselect.png)

Different cross sections can be visualized within the geometry using GetSlicesFromStructuredFieldbyIndices.
:::

### Show Bounding Box

:::{figure-md} fig-add-bbox

![Visualize the mesh’s bounding box.](BasicTutorial_figures/editmeshboundingbox.png)

Visualize the mesh’s bounding box.
:::

Add the **EditMeshBoundingBox** module under **ChangeMesh** ({numref}`fig-add-bbox`). Connect it to the ReadField module and direct the output to the ViewScene module. Execute the network to visualize the bounding box of engine.nhrd. Adjust the size of the bounding box by pressing the + or - buttons under Widget Scale in the EditMeshBoundingBox UI ({numref}`fig-add-bbox2`).

:::{figure-md} fig-add-bbox2

![Change the scale of the mesh’s bounding box using the Scale Widget in the EditMeshBoundingBox UI.](BasicTutorial_figures/editmeshbbox.png)

Change the scale of the mesh’s bounding box using the Scale Widget in the EditMeshBoundingBox UI.
:::

### Isosurface

:::{figure-md} fig-extract

![Extract an isosurface from field.](BasicTutorial_figures/extractiso.png)

Extract an isosurface from field.
:::

Construct an isosurface from the field by instantiating and connecting a **ExtractSimpleIsosurface** module to the ReadField module. The isovalue must be changed within the ExtractSimpleIsosurface UI. Open the field information by clicking on the connection between the ReadField and ExtractSimpleIsosurface and press I to bring up information. Enter a value from within the data range like 120. Visualize the isosurface by connecting it to a new ShowField module ported into the ViewScene module ({numref}`fig-extract`). Execute the network. Color isosurface output geometry by connecting the RescaleColorMap module to the ShowField module ({numref}`fig-change-isoval`). To better view the geometry, turn off the edges within the ShowField UI ({numref}`fig-view-isosurf`).

:::{figure-md} fig-change-isoval

![Change the isovalue within ExtractSimpleIsosurface UI.](BasicTutorial_figures/coloriso.png)

Change the isovalue within ExtractSimpleIsosurface UI.
:::


:::{figure-md} fig-view-isosurf

![Adjusting parameters within the ShowField UI helps to better visualize the isosurface.](BasicTutorial_figures/edgesiso.png)

Adjusting parameters within the ShowField UI helps to better visualize the isosurface.
:::


## Create, Manipulate and Visualize Field

### Create Field

Create and manipulate a structured mesh type in this exercise. Start by creating a lattice volume using **CreateLatVol** module. Assign data at nodes using **CalculateFieldData** module. Connect CalculateFieldData to CreateLatVol. Input the following expression \\( RESULT = sqrt(X * X + Y * Y + Z * Z) \\) to compute data for each node within the CreateFieldData UI.

:::{figure-md} fig-create-new-field

![Create lattice volume field using CreateLatVol module.](BasicTutorial_figures/create.png)

Create lattice volume field using CreateLatVol module.
:::


:::{figure-md} fig-calc-field-data

![Create a new field by inputting an expression into the CreateFieldData UI.](BasicTutorial_figures/fielddata.png)

Create a new field by inputting an expression into the CreateFieldData UI.
:::

### Isosurface

:::{figure-md} fig-extract-isosurf2

![Extract an isosurface from the field data.](BasicTutorial_figures/extractiso2.png)

Extract an isosurface from the field data.
:::

Generate the isosurface by instantiating and connecting an ExtractSimpleIsosurface module to CalculateFieldData ({numref}`fig-extract-isosurf2`). Adjust the isovalue within the ExtractSimpleIsosurface UI so that the isosurface can be visualized ({numref}`fig-conn-colormap`). Add a color map and visualize the isosurface as in the previous [example](#isosurface) ({numref}`fig-view-default-isosurf`). Show the mesh bounding box as in [Show Bounding Box](#show-bounding-box) section ({numref}`fig-change-isoval2`).

:::{figure-md} fig-conn-colormap

![Change the isovalue so that an isosurface can be visualized.](BasicTutorial_figures/selectisoval.png)

Change the isovalue so that an isosurface can be visualized.
:::


:::{figure-md} fig-view-default-isosurf

![Visualize the isosurface.](BasicTutorial_figures/viewisocirc.png)

Visualize the isosurface.
:::


:::{figure-md} fig-change-isoval2

![Visualize the mesh’s bounding box.](BasicTutorial_figures/bbox.png)

Figure 3.6 Visualize the mesh’s bounding box.
:::

### Slice Field

Extend the functionality of this network by slicing the field using GetSliceFromStructuredFieldByIndices as in previous [example](#slice-field).

:::{figure-md} fig-slice-mod2

![Insert GetSliceFromStructuredFieldByIndices into the network.](BasicTutorial_figures/getslice2.png)

Insert GetSliceFromStructuredFieldByIndices into the network.
:::


:::{figure-md} fig-show-slice

![Change the slice index using the GetSliceFromStructuredFieldByIndices UI.](BasicTutorial_figures/changeslice.png)

Change the slice index using the GetSliceFromStructuredFieldByIndices UI.
:::


:::{figure-md} fig-shows-slice

![Attach the RescaleColorMap module to the ShowField module.](BasicTutorial_figures/colorslice.png)

Attach the RescaleColorMap module to the ShowField module.
:::

### Clip Field

:::{figure-md} fig-clip-field-func

![Convert the original field to an unstructured mesh.](BasicTutorial_figures/convertmesh.png)

Convert the original field to an unstructured mesh.
:::

Clip out a subset of the original field by converting the lattice volume to an unstructured mesh using **ConvertMeshToUnstructuredMesh** ({numref}`fig-clip-field-func`) and adding **ClipFieldByFunction** ({numref}`fig-convert-mesh`) to the network. Set the clipping location setting in ClipFieldByFunction to *all nodes*. Use the expression `DATA1 > 1 && X < 0` to clip the field ({numref}`fig-clip-field`).

:::{figure-md} fig-convert-mesh

![Insert a ClipFieldbyFunction module.](BasicTutorial_figures/clipfield.png)

Insert a ClipFieldbyFunction module.
:::


:::{figure-md} fig-clip-field

![Clip the field by entering an expression in the ClipField UI.](BasicTutorial_figures/clipfield_input.png)

Clip the field by entering an expression in the ClipField UI.
:::

#### Extract Boundary

:::{figure-md} fig-clip-field-input

![Build and apply the mapping network connections.](BasicTutorial_figures/mappingmatrix.png)

Build and apply the mapping network connections.
:::

At this point, it will be necessary to map the fields by interpolating the the boundary surface field to the clipping field. First, use **BuildMappingMatrix** to build a matrix that maps a linear combination of data values in the clipping field to a value in the boundary field. Then use **ApplyMappingMatrix** to multiply the data vector of the clipping field with the mapping matrix to obtain the data vector for the boundary surface field ({numref}`fig-clip-field-input`). Use GetFieldBoundary to extract the boundary surface from the lattice volume and use it as input into the ApplyMappingMatrixModule and BuildMapping Matrix ({numref}`fig-mapping-matrix`). Port the output from the BuildMappingMatrix module to ApplyMappingMatrix and visualize the resultant field using a ShowFieldModule ({numref}`fig-highlight-mods`). Add a colormap to and enable transparency in ShowField UI for further functionality ({numref}`fig-view-bound`).

:::{figure-md} fig-mapping-matrix

![Add GetFieldBoundary to the network.](BasicTutorial_figures/fieldboundary.png)

Add GetFieldBoundary to the network.
:::


:::{figure-md} fig-highlight-mods

![Connect all the modules for mapping and visualize the output.](BasicTutorial_figures/view_boundary.png)

Connect all the modules for mapping and visualize the output.
:::


:::{figure-md} fig-view-bound

![Add a colormap and enable transparency.](BasicTutorial_figures/finalview.png)

Add a colormap and enable transparency.
:::


Finally, it is not strictly necessary to explicitly convert the original mesh to an unstructured mesh using ConvertMeshToUnstructuredMesh because ClipFieldByFunction can implicitly convert structured mesh types to unstructured mesh types before clipping the field. As a final exercise, delete ConvertMeshToUnstructuredMesh from the network and try to obtain the same result.
