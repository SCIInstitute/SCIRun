# ClipVolumeByIsovalue

This module clips a scalar field to a specified isovalue.

**Detailed Description**

The ClipVolumeByIsovalue module is used to clip a TetVol, HexVol or TriSurf field along a particular isovalue. The isovalue is specified by the entry in the GUI. The new field can contain either the values less than or greater than the isovalue, as selected by the user.

In order to compute where the cuts are to be made, the input field must contain scalar values at the nodes. If the input field contains element centered data, use modules in the ChangeFieldData category ([MapFieldDataFromSourceToDestination](../ChangeFieldData/MapFieldDataFromSourceToDestination.md), [MapFieldDataOntoNodes](../ChangeFieldData/MapFieldDataOntoNodes.md), [MapFieldDataFromElemToNode](../ChangeFieldData/MapFieldDataFromElemToNode.md), etc.) to interpolate data from elements to nodes.

Only TetVol, HexVol and TriSurf fields (unstructured, irregular mesh types) are supported by this general clipping. Other fields should be converted into these types if they are to be clipped.
