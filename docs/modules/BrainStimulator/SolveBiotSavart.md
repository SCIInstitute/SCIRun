# SolveBiotSavart

Calculation of magnetic field through a given domain mesh with a TMS coil geometry.  

**Detailed Description**

The module calculates the magnetic field due to a given TMS coil geometry and parameters through a domain mesh based on the Biot-Savart law.

Inputs to the module are:
 - Coil (Field type): A TMS coil field, assumed to match the format of the [ModelTMSCoil](ModelTMSCoil.md) module output.  
 - Mesh (Field type): A head or domain mesh for the magnetic field calculation.

Outputs are only generated when ports are connected.  These are:
 - VectorBField (Matrix type): Matrix containing the **magnetic field** vectors for each point on the domain mesh.  Use [SetFieldData](../ChangeFieldData/SetFieldData.md) to reassign these values to the mesh 
 - VectorAField (Matrix type): Matrix containing the **magnetic vector potential** vectors for each point on the domain mesh.  Use [SetFieldData](../ChangeFieldData/SetFieldData.md) to reassign these values to the mesh. 
