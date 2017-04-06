1  SCIRun Overview
==================

This tutorial demonstrates how to build a simple SCIRun dataflow network.

1.1  Software requirements
--------------------------

### 1.1.1  SCIRun 

All available downloads for SCIRun version and the SCIRunData archive are available from [SCI software portal](http://www.scirun.org). Make sure to update to the most up-to-date release available, which will include the latest bug fixes.

Currently, the easiest way to get started with SCIRun version is to download and install a binary version for Mac OS X. Sources are also available for Linux, however this option is recommended only for advanced Linux users.

Unpack the SCIRunData archive in a convenient location. Recall from the User Guide that the path to data can be set using the environment variable or by setting in the *.scirunrc* file.

2  Simple Dataflow Network
=======================

Scope: - - - -

2.1  Slice Field
----------------

The purpose of this section is to read, manipulate, and visualize a structured mesh dataset originating from SCIRunData.

### 2.1.1  Read Data File

Create a **ReadField** module by using the **Module Selector** on the left hand side of the screen. Navigate to **DataIO** subsection using the scroll bar in the Module Selector and instantiate a ReadField (Figure 2.1). Recall from the **User Guide** that a module can also be selected by giving a text input into the filter in the Module Selector (Figure 2.2).

<img src="BasicTutorial_figures/readfield.png" title="Locate ReadField module using scroll bar in the Module Selector." style="width:80.0%" />
Figure 2.1 Locate ReadField module using scroll bar in the Module Selector.

<img src="BasicTutorial_figures/readfield_text.png" title="Locate ReadField module using text input into filter." style="width:80.0%" />
Figure 2.2 Locate ReadField module using text input into filter.

Within the ReadField **user interface (UI)**, click the open button to navigate to the SCIRunData directory and select the dataset *volume/engine.nhdr* (Figure 2.3). Notice that many different file formats can be imported by changing the file type within the ReadField selector window. When using Mac OSX El Capitan, press the options button in the ReadField selector window to change the file type. Change the file type to Nrrd file. The ReadField UI can be closed after selection to provide for a larger network viewing frame.

<img src="BasicTutorial_figures/readfield_select.png" title="The ReadField selector window can be used to select and read many data files." style="width:80.0%" />
Figure 2.3 The ReadField selector window can be used to select and read many data files.

### 2.1.2  Slice Field

Slice the engine field by node index along a given axis by instantiating the module **GetSlicesFromStructuredFieldByIndices** in the **NewField** category and connecting it to ReadField (Figure 2.4). This can be done by using the Module Selector filter or scrolling through the list of modules in the Module Selector.

<img src="BasicTutorial_figures/getslice.png" title="Using the ReadField port’s pop-up module menu to instantiate GetSliceFromStructuredFieldByIndices." style="width:80.0%" />
Figure 2.4 Using the ReadField port’s pop-up module menu to instantiate GetSliceFromStructuredFieldByIndices.

### 2.1.3  Visualize Field

To visualize the field geometry, instantiate module **ShowField** in the **Visualization** category and module **ViewScene** in the **Render** category (Figure 2.5). ShowField takes a field as input, and outputs scene-graph geometry. ViewScene displays the geometry and allows a user to interact with the scene.

<img src="BasicTutorial_figures/viewscene.png" title="SCIRun can be used to visualize the structured mesh." style="width:80.0%" />
Figure 2.5 SCIRun can be used to visualize the structured mesh.

Apply a colored scale to the data values on the geometry using **CreateStandardColorMaps** and **RescaleColorMaps** modules in **Visualization** (Figure 2.6). Colors can be manipulated using the CreateStandardColorMap UI and RescaleColorMap UI (Figure 2.7). Change the coloring scheme to Blackbody using the drop-down menu in the CreatSrandardColorMap UI.

<img src="BasicTutorial_figures/colorscale.png" title="Apply and rescale a colormap to data values on the geometry." style="width:80.0%" />
Figure 2.6 Apply and rescale a colormap to data values on the geometry.

<img src="BasicTutorial_figures/manipulatecolorscale.png" title="Manipulate the color scaling using both the CreateStandardColorMaps and RescaleColorMaps modules." style="width:80.0%" />
Figure 2.7 Manipulate the color scaling using both the CreateStandardColorMaps and RescaleColorMaps modules.

Return to the default color scale. Use the sliders in the GetSlicesFromStructuredFieldByIndices UI to change slice position within the geometry. Compare with figure 2.6.

<img src="BasicTutorial_figures/sliceselect.png" title="Different cross sections can be visualized within the geometry using GetSlicesFromStructuredFieldbyIndices. " style="width:90.0%" />
Figure 2.8 Different cross sections can be visualized within the geometry using GetSlicesFromStructuredFieldbyIndices. 

2.2  Show Bounding Box
----------------------

Add the **EditMeshBoundingBox** module under **ChangeMesh** (Figure 2.9). Connect it to the ReadField module and direct the output to the ViewScene module. Execute the network to visualize the bounding box of engine.nhrd. Adjust the size of the bounding box by pressing the + or - buttons under Widget Scale in the EditMeshBoundingBox UI (Figure 2.10).

<img src="BasicTutorial_figures/editmeshboundingbox.png" title="Visualize the mesh’s bounding box." style="width:90.0%" />
Figure 2.9 Visualize the mesh’s bounding box.

<img src="BasicTutorial_figures/editmeshbbox.png" title="Change the scale of the mesh’s bounding box using the Scale Widget in the EditMeshBoundingBox UI." style="width:90.0%" />
Figure 2.10 Change the scale of the mesh’s bounding box using the Scale Widget in the EditMeshBoundingBox UI.

2.3  Isosurface
---------------

Construct an isosurface from the field by instantiating and connecting a **ExtractSimpleIsosurface** module to the ReadField module. The isovalue must be changed within the ExtractSimpleIsosurface UI. Open the field information by clicking on the connection between the ReadField and ExtractSimpleIsosurface and press I to bring up information. Enter a value from within the data range like 120. Visualize the isosurface by connecting it to a new ShowField module ported into the ViewScene module (Figure 2.11). Execute the network. Color isosurface output geometry by connecting the RescaleColorMap module to the ShowField module (Figure 2.12). To better view the geometry, turn off the edges within the ShowField UI (Figure 2.13).

<img src="BasicTutorial_figures/extractiso.png" title="Extract an isosurface from field." style="width:90.0%" />
Figure 2.11 Extract an isosurface from field.

<img src="BasicTutorial_figures/coloriso.png" title="Change the isovalue within ExtractSimpleIsosurface UI." style="width:90.0%" />
Figure 2.12 Change the isovalue within ExtractSimpleIsosurface UI.

<img src="BasicTutorial_figures/edgesiso.png" title="Adjusting parameters within the ShowField UI helps to better visualize the isosurface." style="width:90.0%" />
Figure 2.13 Adjusting parameters within the ShowField UI helps to better visualize the isosurface.

3  Create, Manipulate and Visualize Field
======================================

Scope: - -

3.1  Create Field
-----------------

Create and manipulate a structured mesh type in this exercise. Start by creating a lattice volume using **CreateLatVol** module. Assign data at nodes using **CalculateFieldData** module. Connect CalculateFieldData to CreateLatVol. Input the expression *R**E**S**U**L**T* = *s**q**r**t*(*X* \* *X* + *Y* \* *Y* + *Z* \* *Z*) to compute data for each node within the CreateFieldData UI.

<img src="BasicTutorial_figures/create.png" title="Create lattice volume field using CreateLatVol module." style="width:90.0%" />
Figure 3.1 Create lattice volume field using CreateLatVol module.

<img src="BasicTutorial_figures/fielddata.png" title="Create a new field by inputting an expression into the CreateFieldData UI. " style="width:90.0%" />
Figure 3.2 Create a new field by inputting an expression into the CreateFieldData UI.

3.2  Isosurface
---------------

Generate the isosurface by instantiating and connecting an ExtractSimpleIsosurface module to CalculateFieldData (Figure 3.3). Adjust the isovalue within the ExtractSimpleIsosurface UI so that the isosurface can be visualized (Figure 3.4). Add a color map and visualize the isosurface as in section 2.3 (Figure 3.5). Show the mesh bounding box as in section 2.2 (Figure 3.6).

<img src="BasicTutorial_figures/extractiso2.png" title="Extract an isosurface from the field data." style="width:90.0%" />
Figure 3.3 Extract an isosurface from the field data.

<img src="BasicTutorial_figures/selectisoval.png" title="Change the isovalue so that an isosurface can be visualized." style="width:90.0%" />
Figure 3.4 Change the isovalue so that an isosurface can be visualized.

<img src="BasicTutorial_figures/viewisocirc.png" title="Visualize the isosurface." style="width:90.0%" />
Figure 3.5 Visualize the isosurface.

<img src="BasicTutorial_figures/bbox.png" title="Visualize the mesh’s bounding box." style="width:90.0%" />
Figure 3.6 Visualize the mesh’s bounding box.

3.3  Slice Field
----------------

Extend the functionality of this network by slicing the field using GetSliceFromStructuredFieldByIndices as in section 2.1.2.

<img src="BasicTutorial_figures/getslice2.png" title="Insert GetSliceFromStructuredFieldByIndices into the network." style="width:90.0%" />
Figure 3.7 Insert GetSliceFromStructuredFieldByIndices into the network.

<img src="BasicTutorial_figures/changeslice.png" title="Change the slice index using the GetSliceFromStructuredFieldByIndices UI." style="width:90.0%" />
Figure 3.8 Change the slice index using the GetSliceFromStructuredFieldByIndices UI.

<img src="BasicTutorial_figures/colorslice.png" title="Attach the RescaleColorMap module to the ShowField module." style="width:90.0%" />
Figure 3.9 Attach the RescaleColorMap module to the ShowField module.

3.4  Clip Field
---------------

Clip out a subset of the original field by converting the lattice volume to an unstructured mesh using **ConvertMeshToUnstructuredMesh** (Figure 3.10) and adding **ClipFieldByFunction** (Figure 3.11) to the network. Set the clipping location setting in ClipFieldByFunction to *all nodes*. Use the expression *D**A**T**A*1 &gt; 1&&*X* &lt; 0 to clip the field (Figure 3.12).

<img src="BasicTutorial_figures/convertmesh.png" title="Convert the original field to an unstructured mesh." style="width:90.0%" />
Figure 3.10 Convert the original field to an unstructured mesh.

<img src="BasicTutorial_figures/clipfield.png" title="Insert a ClipFieldbyFunction module." style="width:90.0%" />
Figure 3.11 Insert a ClipFieldbyFunction module.

<img src="BasicTutorial_figures/clipfield_input.png" title="Clip the field by entering an expression in the ClipField UI." style="width:90.0%" />
Figure 3.12 Clip the field by entering an expression in the ClipField UI.

### 3.4.1  Extract Boundary

At this point, it will be necessary to map the fields by interpolating the the boundary surface field to the clipping field. First, use **BuildMappingMatrix** to build a matrix that maps a linear combination of data values in the clipping field to a value in the boundary field. Then use **ApplyMappingMatrix** to multiply the data vector of the clipping field with the mapping matrix to obtain the data vector for the boundary surface field (Figure 3.13). Use GetFieldBoundary to extract the boundary surface from the lattice volume and use it as input into the ApplyMappingMatrixModule and BuildMapping Matrix (Figure 3.14). Port the output from the BuildMappingMatrix module to ApplyMappingMatrix and visualize the resultant field using a ShowFieldModule (Figure 3.15). Add a colormap to and enable transparency in ShowField UI for further functionality (Figure 3.16)

<img src="BasicTutorial_figures/mappingmatrix.png" title="Build and apply the mapping network connections." style="width:90.0%" />
Figure 3.13 Build and apply the mapping network connections.

<img src="BasicTutorial_figures/fieldboundary.png" title="Add GetFieldBoundary to the network." style="width:90.0%" />
Figure 3.14 Add GetFieldBoundary to the network.

<img src="BasicTutorial_figures/view_boundary.png" title="Connect all the modules for mapping and visualize the output." style="width:90.0%" />
Figure 3.15 Connect all the modules for mapping and visualize the output.

<img src="BasicTutorial_figures/finalview.png" title="Add a colormap and enable transparency." style="width:90.0%" />
Figure 3.16 Add a colormap and enable transparency.

Finally, it is not strictly necessary to explicitly convert the original mesh to an unstructured mesh using ConvertMeshToUnstructuredMesh because ClipFieldByFunction can implicitly convert structured mesh types to unstructured mesh types before clipping the field. As a final exercise, delete ConvertMeshToUnstructuredMesh from the network and try to obtain the same result.
