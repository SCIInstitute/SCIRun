# SolveBioSavart

Calculation of magnetic field through a given domain mesh with a TMS coil geometry.  

**Detailed Description**

The modules calculates the magnetic field due to given TMS coil geometry and parameters at through a provide domain mesh based on the Biot-Savart law.

See the [Brain Stimulator Toolkit](https://github.com/SCIInstitute/BrainStimulator) and its [Manual](https://github.com/SCIInstitute/BrainStimulator/blob/master/Documentation/Manuals/BrainStimulatorTutorial.pdf) for more information.  

Inputs to the module are:
 - Coil (Field type): a TMS coil field, assumed to match the format of the [ModelTMSCoil](ModelTMSCoil.md) module output.  
 - Mesh [Field type]: head or domain mesh for the magnetic field prediction

Outputs are only generated when ports are connected.  These are:
 - VectorBField (Matrix type): Matrix containing the **magnetic field** values for each point on the domain mesh.  Use [SetFieldData](../ChangeFieldData/SetFieldData.md) to reassign these values to the mesh 
 - VectorAField (Matrix type): Matrix containing the **magnetic vector potential** values for each point on the domain mesh.  Use [SetFieldData](../ChangeFieldData/SetFieldData.md) to reassign these values to the mesh. 
