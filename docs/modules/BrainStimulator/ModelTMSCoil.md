# ModelTMSCoil

Generate a TMS coil geometry with current sources to model include in TMS modeling pipeline.  

**Detailed Description**

The ModelTMSCoil module will use a set of parameters from the module UI (or module state) to generate mesh with field data to represent a TMS stimulation device in a TMS modeling pipeline.  This module is designed to work with the [SolveBioSavart](SolveBioSavart.md) module where the current sources of the coil will be used to estimate the magnetic field through a target domain.  

See the [Brain Stimulator Toolkit](https://github.com/SCIInstitute/BrainStimulator) and its [Manual](https://github.com/SCIInstitute/BrainStimulator/blob/master/Documentation/Manuals/BrainStimulatorTutorial.pdf) for more information.  

Module parameters used to generate the TMS coil geometry and field data are:
 - 

Inputs to the module are:
 - Coil (Field type): a TMS coil field, assumed to match the format of the [ModelTMSCoil](ModelTMSCoil.md) module output.  
 - Mesh [Field type]: head or domain mesh for the magnetic field prediction

Outputs are only generated when ports are connected.  These are:
 - VectorBField (Matrix type): Matrix containing the **magnetic field** values for each point on the domain mesh.  Use [SetFieldData](../ChangeFieldData/SetFieldData.md) to reassign these values to the mesh 
 - VectorAField (Matrix type): Matrix containing the **magnetic vector potential** values for each point on the domain mesh.  Use [SetFieldData](../ChangeFieldData/SetFieldData.md) to reassign these values to the mesh. 
