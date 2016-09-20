SCIRun Overview
===============

This tutorial demonstrates how to build a simple SCIRun dataflow network.

Software requirements
---------------------

### SCIRun 

All available downloads for SCIRun version and the SCIRunData archive are available from [SCI software portal](http://www.scirun.org). Make sure to update to the most up-to-date release available, which will include the latest bug fixes.

Currently, the easiest way to get started with SCIRun version is to download and install a binary version for Mac OS X. Sources are also available for Linux, however this option is recommended only for advanced Linux users.

Unpack the SCIRunData archive in a convenient location. Recall from the User Guide that the path to data can be set using the environment variable or by setting in the *.scirunrc* file.

Simple Dataflow Network
=======================

Scope: - - - -

Slice Field
-----------

The purpose of this section is to read, manipulate, and visualize a structured mesh dataset originating from SCIRunData.

### Read Data File

Create a **ReadField** module by using the **Module Selector** on the left hand side of the screen. Navigate to **DataIO** subsection using the scroll bar in the Module Selector and instantiate a ReadField (Figure \[fig:readfield\]). Recall from the **User Guide** that a module can also be selected by giving a text input into the filter in the Module Selector (Figure \[fig:readfield2\]).

<img src="BasicTutorial_figures/readfield_text.png" alt="Locate ReadField module using scroll bar in the Module Selector." style="width:80.0%" />

<img src="BasicTutorial_figures/readfield.png" alt="Locate ReadField module using text input into filter." style="width:80.0%" />

Within the ReadField **user interface (UI)**, click the open button to navigate to the SCIRunData directory and select the dataset *volume/engine.nhdr* (Figure \[fig:readfieldtype\]). Notice that many different file formats can be imported by changing the file type within the ReadField selector window. When using Mac OSX El Capitan, press the options button in the ReadField selector window to change the file type. Change the file type to Nrrd file. The ReadField UI can be closed after selection to provide for a larger network viewing frame.

<img src="BasicTutorial_figures/readfield_select.png" alt="The ReadField selector window can be used to select and read many data files." style="width:80.0%" />

### Slice Field

Slice the engine field by node index along a given axis by instantiating the module **GetSlicesFromStructuredFieldByIndices** in the **NewField** category and connecting it to ReadField (Figure \[fig:slicemod\]). This can be done by using the Module Selector filter or scrolling through the list of modules in the Module Selector.

<img src="BasicTutorial_figures/getslice.png" alt="Using the ReadField port’s pop-up module menu to instantiate GetSliceFromStructuredFieldByIndices." style="width:80.0%" />

### Visualize Field

To visualize the field geometry, instantiate module **ShowField** in the **Visualization** category and module **ViewScene** in the **Render** category (Figure \[fig:colormap\]). ShowField takes a field as input, and outputs scene-graph geometry. ViewScene displays the geometry and allows a user to interact with the scene.

<img src="BasicTutorial_figures/viewscene.png" alt="SCIRun can be used to visualize the structured mesh." style="width:80.0%" />

Apply a colored scale to the data values on the geometry using **CreateStandardColorMaps** and **RescaleColorMaps** modules in **Visualization** (Figure \[fig:rescale\]). Colors can be manipulated using the CreateStandardColorMap UI and RescaleColorMap UI (Figure \[fig:viewscene\]). Change the coloring scheme to Blackbody using the drop-down menu in the CreatSrandardColorMap UI.

<img src="BasicTutorial_figures/colorscale.png" alt="Apply and rescale a colormap to data values on the geometry." style="width:80.0%" />

<img src="BasicTutorial_figures/manipulatecolorscale.png" alt="Manipulate the color scaling using both the CreateStandardColorMaps and RescaleColorMaps modules." style="width:80.0%" />

Return to the default color scale. Use the sliders in the GetSlicesFromStructuredFieldByIndices UI to change slice position within the geometry. Compare with figure \[fig:rescale\].

<img src="BasicTutorial_figures/sliceselect.png" alt="Different cross sections can be visualized within the geometry using GetSlicesFromStructuredFieldbyIndices. " style="width:90.0%" />

Show Bounding Box
-----------------

Add the **EditMeshBoundingBox** module under **ChangeMesh** (Figure \[fig:addbbox\]). Connect it to the ReadField module and direct the output to the ViewScene module. Execute the network to visualize the bounding box of engine.nhrd. Adjust the size of the bounding box by pressing the + or - buttons under Widget Scale in the EditMeshBoundingBox UI (Figure \[fig:addbbox2\]).

<img src="BasicTutorial_figures/editmeshboundingbox.png" alt="Visualize the mesh’s bounding box." style="width:90.0%" />

<img src="BasicTutorial_figures/editmeshbbox.png" alt="Change the scale of the mesh’s bounding box using the Scale Widget in the EditMeshBoundingBox UI." style="width:90.0%" />

Isosurface
----------

Construct an isosurface from the field by instantiating and connecting a **ExtractSimpleIsosurface** module to the ReadField module. The isovalue must be changed within the ExtractSimpleIsosurface UI. Open the field information by clicking on the connection between the ReadField and ExtractSimpleIsosurface and press I to bring up information. Enter a value from within the data range like 120. Visualize the isosurface by connecting it to a new ShowField module ported into the ViewScene module (Figure \[fig:extract\]). Execute the network. Color isosurface output geometry by connecting the RescaleColorMap module to the ShowField module (Figure \[fig:changeisoval\]). To better view the geometry, turn off the edges within the ShowField UI (Figure \[fig:viewisosurf\]).

<img src="BasicTutorial_figures/extractiso.png" alt="Extract an isosurface from field." style="width:90.0%" />

<img src="BasicTutorial_figures/coloriso.png" alt="Change the isovalue within ExtractSimpleIsosurface UI." style="width:90.0%" />

<img src="BasicTutorial_figures/edgesiso.png" alt="Adjusting parameters within the ShowField UI helps to better visualize the isosurface." style="width:90.0%" />

Create, Manipulate and Visualize Field
======================================

Scope: - -

Create Field
------------

Create and manipulate a structured mesh type in this exercise. Start by creating a lattice volume using **CreateLatVol** module. Assign data at nodes using **CalculateFieldData** module. Connect CalculateFieldData to CreateLatVol. Input the expression *R**E**S**U**L**T* = *s**q**r**t*(*X* \* *X* + *Y* \* *Y* + *Z* \* *Z*) to compute data for each node within the CreateFieldData UI.

<img src="BasicTutorial_figures/create.png" alt="Create lattice volume field using CreateLatVol module." style="width:90.0%" />

<img src="BasicTutorial_figures/fielddata.png" alt="Create a new field by inputting an expression into the CreateFieldData UI. " style="width:90.0%" />

Isosurface
----------

Generate the isosurface by instantiating and connecting an ExtractSimpleIsosurface module to CalculateFieldData (Figure \[fig:extractisosurf2\]). Adjust the isovalue within the ExtractSimpleIsosurface UI so that the isosurface can be visualized (Figure \[fig:conncolormap\]). Add a color map and visualize the isosurface as in section \[isosurface\] (Figure \[fig:viewdefaultisosurf\]). Show the mesh bounding box as in section \[bbox\] (Figure \[fig:changeisoval2\]).

<img src="BasicTutorial_figures/extractiso2.png" alt="Extract an isosurface from the field data." style="width:90.0%" />

<img src="BasicTutorial_figures/selectisoval.png" alt="Change the isovalue so that an isosurface can be visualized." style="width:90.0%" />

<img src="BasicTutorial_figures/viewisocirc.png" alt="Visualize the isosurface." style="width:90.0%" />

<img src="BasicTutorial_figures/bbox.png" alt="Visualize the mesh’s bounding box." style="width:90.0%" />

Slice Field
-----------

Extend the functionality of this network by slicing the field using GetSliceFromStructuredFieldByIndices as in section \[slice\].

<img src="BasicTutorial_figures/getslice2.png" alt="Insert GetSliceFromStructuredFieldByIndices into the network." style="width:90.0%" />

<img src="BasicTutorial_figures/changeslice.png" alt="Change the slice index using the GetSliceFromStructuredFieldByIndices UI." style="width:90.0%" />

<img src="BasicTutorial_figures/colorslice.png" alt="Attach the RescaleColorMap module to the ShowField module." style="width:90.0%" />

Clip Field
----------

Clip out a subset of the original field by converting the lattice volume to an unstructured mesh using **ConvertMeshToUnstructuredMesh** (Figure \[fig:convertmesh\]) and adding **ClipFieldByFunction** (Figure \[fig:clipfield\]) to the network. Set the clipping location setting in ClipFieldByFunction to *all nodes*. Use the expression *D**A**T**A*1 &gt; 1&&*X* &lt; 0 to clip the field (Figure \[fig:clipfield\_input\]).

<img src="BasicTutorial_figures/convertmesh.png" alt="Convert the original field to an unstructured mesh." style="width:90.0%" />

<img src="BasicTutorial_figures/clipfield.png" alt="Insert a ClipFieldbyFunction module." style="width:90.0%" />

<img src="BasicTutorial_figures/clipfield_input.png" alt="Clip the field by entering an expression in the ClipField UI." style="width:90.0%" />

### Extract Boundary

At this point, it will be necessary to map the fields by interpolating the the boundary surface field to the clipping field. First, use **BuildMappingMatrix** to build a matrix that maps a linear combination of data values in the clipping field to a value in the boundary field. Then use **ApplyMappingMatrix** to multiply the data vector of the clipping field with the mapping matrix to obtain the data vector for the boundary surface field (Figure \[fig:mappingmatrix\]). Use GetFieldBoundary to extract the boundary surface from the lattice volume and use it as input into the ApplyMappingMatrixModule and BuildMapping Matrix (Figure \[fig:highlightmods\]). Port the output from the BuildMappingMatrix module to ApplyMappingMatrix and visualize the resultant field using a ShowFieldModule (Figure \[fig:viewbound\]). Add a colormap to and enable transparency in ShowField UI for further functionality (Figure \[fig:finalview\])

<img src="BasicTutorial_figures/mappingmatrix.png" alt="Build and apply the mapping network connections." style="width:90.0%" />

<img src="BasicTutorial_figures/fieldboundary.png" alt="Add GetFieldBoundary to the network." style="width:90.0%" />

<img src="BasicTutorial_figures/view_boundary.png" alt="Connect all the modules for mapping and visualize the output." style="width:90.0%" />

<img src="BasicTutorial_figures/finalview.png" alt="Add a colormap and enable transparency." style="width:90.0%" />

Finally, it is not strictly necessary to explicitly convert the original mesh to an unstructured mesh using ConvertMeshToUnstructuredMesh because ClipFieldByFunction can implicitly convert structured mesh types to unstructured mesh types before clipping the field. As a final exercise, delete ConvertMeshToUnstructuredMesh from the network and try to obtain the same result.
