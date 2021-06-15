# CalculateCurrentDensity

Compute the current density vector field

**Detailed Description**

CalculateCurrentDensity calculates the current density vector field.

Required inputs are the electric field and mesh with conductivities.

Technical note: The current density vector field J is the product of sigma and -del V. The minus sign is added in CalculateCurrentDensity, so the electric field input should be positive (which is the unmodified output of the [CalculateGradients](../ChangeFieldData/CalculateGradients.md) module).
